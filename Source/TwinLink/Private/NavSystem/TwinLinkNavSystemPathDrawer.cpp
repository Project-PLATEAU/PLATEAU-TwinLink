// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "NavSystem/TwinLinkNavSystemPathDrawer.h"

#include "ImathMath.h"
#include "ImathMath.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraComponent.h"
// Sets default values for this component's properties
AUTwinLinkNavSystemPathDrawer::AUTwinLinkNavSystemPathDrawer() {
}

AUTwinLinkNavSystemPathDrawerNiagara::AUTwinLinkNavSystemPathDrawerNiagara() {
}

void AUTwinLinkNavSystemPathDrawerNiagara::DrawPath(const TArray<FVector>& PathPoints) {
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
    //UNiagaraDataInterface::Set
}

AUTwinLinkNavSystemPathDrawerDebug::AUTwinLinkNavSystemPathDrawerDebug() {
    PrimaryActorTick.bCanEverTick = true;
}

void AUTwinLinkNavSystemPathDrawerDebug::DrawPath(const TArray<FVector>& PathPoints) {
    Super::DrawPath(PathPoints);
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

