// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "NavSystem/TwinLinkNavSystemPathDrawer.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "TwinLinkCommon.h"
// Sets default values for this component's properties
AUTwinLinkNavSystemPathDrawer::AUTwinLinkNavSystemPathDrawer() {
}

AUTwinLinkNavSystemPathDrawerNiagara::AUTwinLinkNavSystemPathDrawerNiagara() {
    PrimaryActorTick.bCanEverTick = true;
}

void AUTwinLinkNavSystemPathDrawerNiagara::DrawPath(const TArray<FVector>& PathPoints, float DeltaSeconds) {
    Super::DrawPath(PathPoints, DeltaSeconds);

    const auto NiagaraComponent = GetComponentByClass<UNiagaraComponent>();
    checkf(NiagaraComponent != nullptr, TEXT("NiagaraSystem is nll"));
    if (!NiagaraComponent)
        return;

    auto Length = 0.f;
    for (auto I = 1; I < PathPoints.Num(); ++I)
        Length += (PathPoints[I] - PathPoints[I - 1]).Length();
    const auto ParticleNum = static_cast<int>(FMath::Max(1, Length / DrawPointInterval));
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, "PathPoints", PathPoints);
    NiagaraComponent->SetNiagaraVariableInt("ParticleNum", ParticleNum);
    NiagaraComponent->SetNiagaraVariableVec3("Offset", FVector::UpVector * DrawPointHeightOffset);
    NiagaraComponent->SetNiagaraVariableFloat("NightCoef", TwinLinkGraphicsEnv::GetNightIntensity(GetWorld()));
}

void AUTwinLinkNavSystemPathDrawerNiagara::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
#if WITH_EDITOR
    if (DebugNightCoef >= 0.f) {
        const auto NiagaraComponent = GetComponentByClass<UNiagaraComponent>();
        NiagaraComponent->SetNiagaraVariableFloat("NightCoef", DebugNightCoef);
        DebugNightCoef = -1.f;
    }
#endif
}

AUTwinLinkNavSystemPathDrawerArrow::AUTwinLinkNavSystemPathDrawerArrow() {
    PrimaryActorTick.bCanEverTick = true;
}

void AUTwinLinkNavSystemPathDrawerArrow::BeginPlay() {
    Super::BeginPlay();
    const auto NightIntensity = TwinLinkGraphicsEnv::GetNightIntensity(GetWorld());
    for (auto& P : GetComponentsByClass(UStaticMeshComponent::StaticClass())) {
        if (auto StaMesh = Cast<UStaticMeshComponent>(P)) {
            ArrowComps.Add(StaMesh);
            auto Mat = StaMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, StaMesh->GetMaterial(0));
            Mat->SetScalarParameterValue(FName(TEXT("NightCoef")), NightIntensity);
        }
    }
}

void AUTwinLinkNavSystemPathDrawerArrow::DrawPath(const TArray<FVector>& PathPoints, float DeltaSeconds) {
    Super::DrawPath(PathPoints, DeltaSeconds);

    DrawPathPoints = PathPoints;
    TotalPathDistance = 0.f;
    for (auto I = 0; I < (PathPoints.Num() - 1); ++I) {
        TotalPathDistance += (PathPoints[I + 1] - PathPoints[I]).Length();
    }
    NowPosition = 0;
    UpdateMatrix(0.f, false);
}

bool AUTwinLinkNavSystemPathDrawerArrow::UpdateMatrix(float DeltaSeconds, bool bInterpolate) {
    const auto Num = FMath::Min(MaxArrowNum, FMath::CeilToInt(TotalPathDistance / DrawPointInterval));
    // 必要分を作る(作れなかった時のためにWhileではなくforで)
    for (auto I = ArrowComps.Num(); I < Num; ++I)
        CreateChildArrow();

    for (auto I = 0; I < ArrowComps.Num(); ++I) {
        const auto Arrow = ArrowComps[I];
        if (!Arrow)
            continue;
        Arrow->SetVisibility(I < Num);
    }
    if (Num <= 0)
        return false;

    // 最高速度を確認する
    const auto MaxSpeed = TotalPathDistance / FMath::Max(0.1f, MinMoveSec);
    const auto Speed = FMath::Min(MaxSpeed, MoveSpeed);
    NowPosition = FMath::Fmod(NowPosition + Speed * DeltaSeconds, TotalPathDistance);
    TArray<std::pair<FVector, FVector>> Locations;
    for (auto J = 0; J < Num; ++J) {
        const auto Cmp = ArrowComps[J];
        if (!Cmp)
            continue;
        // 等間隔に配置する
        const auto Offset = TotalPathDistance * (1.f * J / Num);
        auto Pos = Offset + NowPosition;
        if (Pos >= TotalPathDistance)
            Pos -= TotalPathDistance;

        auto LastPos = 0.f;
        for (auto I = 0; I < (DrawPathPoints.Num() - 1); ++I) {

            auto& St = DrawPathPoints[I];
            auto& En = DrawPathPoints[I + 1];
            auto Len = (En - St).Length();
            if (Len <= 1e-5f)
                continue;
            if (LastPos <= Pos && Pos <= (LastPos + Len)) {
                auto T = 1.f - (LastPos + Len - Pos) / Len;
                auto P = FMath::Lerp(St, En, T);
                auto RotMat = FRotationMatrix::MakeFromXZ((En - St).GetSafeNormal(), FVector::UpVector).ToQuat();
                Cmp->SetWorldLocation(P + FVector::UpVector * DrawPointHeightOffset);
                auto NowRot = Cmp->K2_GetComponentRotation();
                auto RotT = FMath::Min(1.f, RotSpeedCoef * DeltaSeconds);
                // 1周した時に変に回転しないようにI==0の時は補完しない
                if (bInterpolate == false || I == 0)
                    RotT = 1.f;
                auto NewRot = FMath::Lerp(FQuat(NowRot), RotMat, RotT);
                Cmp->SetWorldRotation(NewRot);
            }
            LastPos += Len;
        }
    }
    return true;
}

void AUTwinLinkNavSystemPathDrawerArrow::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    UpdateMatrix(DeltaSeconds, true);
    if (DebugNightCoef >= 0.f) {
        DebugNightCoef = -1.f;
        for (const auto Arrow : ArrowComps) {
            if (const auto Mat = Cast<UMaterialInstanceDynamic>(Arrow->GetMaterial(0)))
                Mat->SetScalarParameterValue("NightCoef", DebugNightCoef);
        }
    }
}

UActorComponent* AUTwinLinkNavSystemPathDrawerArrow::CreateChildArrow() {
    FName Name = FName(FString::Printf(TEXT("Arrow%d"), ArrowComps.Num()));

    //auto Comp = AddComponentByClass(ArrowTemplate, false, FTransform::Identity, false);// CreateDefaultSubobject(Name, UStaticMeshComponent::StaticClass(), ArrowTemplate.Get(), false, false);
    auto Comp = NewObject<UActorComponent>(this, ArrowTemplate);
    auto StaMesh = Cast<UStaticMeshComponent>(Comp);
    check(StaMesh);
    if (!StaMesh)
        return nullptr;
    StaMesh->RegisterComponent();
    StaMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    ArrowComps.Add(StaMesh);
    return StaMesh;
}

AUTwinLinkNavSystemPathDrawerDebug::AUTwinLinkNavSystemPathDrawerDebug() {
    PrimaryActorTick.bCanEverTick = true;
}

void AUTwinLinkNavSystemPathDrawerDebug::DrawPath(const TArray<FVector>& PathPoints, float DeltaSeconds) {
    Super::DrawPath(PathPoints, DeltaSeconds);
    DebugPathPoints = PathPoints;
}

void AUTwinLinkNavSystemPathDrawerDebug::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    auto& points = DebugPathPoints;
    FVector LastPos = FVector::Zero();
    for (auto i = 0; i < points.Num(); ++i) {
        auto Pos = points[i] + FVector::UpVector * DebugFindPathUpOffset;
        DrawDebugSphere(GetWorld(), Pos, 5, 10, FColor::Red, false);
        if (i > 0) {
            DrawDebugLine(GetWorld(), LastPos, Pos, FColor::Blue);
            /* if(i <= FindPathLineActorArray.Num())
             {
                 auto line = FindPathLineActorArray[i - 1];
                 line->SetStartPosition(LastPos);
                 line->SetEndPosition(Pos);
                 line->SetStartAndEnd(LastPos, FVector::Zero(), Pos, FVector::Zero(), true);
             }*/
        }
        LastPos = Pos;
    }
}

