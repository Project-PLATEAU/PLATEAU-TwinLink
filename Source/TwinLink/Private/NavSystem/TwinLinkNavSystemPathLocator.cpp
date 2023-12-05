// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "NavSystem/TwinLinkNavSystemPathLocator.h"

#include "NavigationSystem.h"
#include "TwinLink.h"
#include "TwinLinkCommon.h"
#include "Misc/TwinLinkMathEx.h"
#include "TwinLinkWorldViewer.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/TwinLinkActorEx.h"
#include "NavSystem/TwinLinkNavSystem.h"

// Sets default values
ATwinLinkNavSystemPathLocator::ATwinLinkNavSystemPathLocator() {
    PrimaryActorTick.bCanEverTick = true;
}

void ATwinLinkNavSystemPathLocator::BeginPlay() {
    Super::BeginPlay();

    const auto NightIntensity = TwinLinkGraphicsEnv::GetNightIntensity(GetWorld());
    TArray<UActorComponent*> Comps;
    GetComponents(UStaticMeshComponent::StaticClass(), Comps);
    for (const auto P : Comps) {
        if (const auto StaMesh = Cast<UStaticMeshComponent>(P)) {
            const auto Mat = StaMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, StaMesh->GetMaterial(0));
            Mat->SetScalarParameterValue(FName(TEXT("NightCoef")), NightIntensity);
        }
    }
}

void ATwinLinkNavSystemPathLocator::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    if (auto WorldViewer = ATwinLinkNavSystem::GetWorldViewer(GetWorld())) {
        const auto Forward = WorldViewer->GetNowCameraRotationOrDefault().RotateVector(FVector::ForwardVector);
        const auto ActorLocation = GetActorLocation();
        // #NOTE : モデルがYForwardなのでそれに合わせる
        const auto Rotation = FRotationMatrix::MakeFromYZ(-Forward, FVector::UpVector).ToQuat();
        SetActorRotation(Rotation);
    }

    {
        FVector2D ScreenPos;
        TwinLinkNavSystemPathLocatorState S;
        auto WarTextVisible = TryGetShowWarnText(ScreenPos, S);
        if (WarTextVisible) {
            if (!WarnTextWidget && WarnTextWidgetBp) {
                APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
                WarnTextWidget = CreateWidget(GetWorld(), WarnTextWidgetBp);
                WarnTextWidget->AddToViewport(1);
            }
        }

        if (WarnTextWidget) {
            WarnTextWidget->SetVisibility(UTwinLinkBlueprintLibrary::AsSlateVisibility(WarTextVisible));
            if (WarTextVisible)
                WarnTextWidget->SetPositionInViewport(ScreenPos);
        }
    }

#if WITH_EDITOR
    if (DebugNightCoef >= 0.f) {
        TArray<UActorComponent*> Comps;
        GetComponents(UStaticMeshComponent::StaticClass(), Comps);
        for (const auto P : Comps) {
            if (const auto StaMesh = Cast<UStaticMeshComponent>(P)) {
                const auto Mat = StaMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, StaMesh->GetMaterial(0));
                Mat->SetScalarParameterValue(FName(TEXT("NightCoef")), DebugNightCoef);
            }
        }
        DebugNightCoef = -1.f;
    }
#endif
}

TwinLinkNavSystemPathLocatorState ATwinLinkNavSystemPathLocator::GetNowState() const {
    return State;
}

bool ATwinLinkNavSystemPathLocator::TryGetShowWarnText(FVector2D& OutScreenPos,
    TwinLinkNavSystemPathLocatorState& OutState) {
    // 範囲外にあるときにウィジットを出す
    OutState = GetNowState();

    // 不正値は無視
    if (FTwinLinkEnumT(OutState).IsValid() == false)
        return false;
    // 正しく道の上にあるときも無視
    if (OutState == TwinLinkNavSystemPathLocatorState::Valid)
        return false;

    if (IsHidden())
        return false;
    const auto P = GetWarnTextWorldLocation();
    const APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    return UGameplayStatics::ProjectWorldToScreen(Controller, P, OutScreenPos);
}

FVector ATwinLinkNavSystemPathLocator::GetWarnTextWorldLocation_Implementation() const {
    const auto Bb = CalculateComponentsBoundingBoxInLocalSpace(false, true);
    const auto Transform = GetActorTransform();
    const auto Up = Transform.GetScaledAxis(EAxis::Z);
    // パディングとして50cm入れておく
    return Up * (Bb.GetSize().Z + 50) + GetActorLocation();
}

bool ATwinLinkNavSystemPathLocator::UpdateLocation(const UNavigationSystemV1* NavSys, const FHitResult& HitResult) {
    SetActorLocation(HitResult.Location);

    // 壁についている
    if (FVector::DotProduct(HitResult.Normal, FVector::UpVector) < FMath::Cos(FMath::DegreesToRadians(70))) {
        State = TwinLinkNavSystemPathLocatorState::OnWall;
        return false;
    }

    return UpdateLocation(NavSys, HitResult.Location);
}

bool ATwinLinkNavSystemPathLocator::UpdateLocation(const UNavigationSystemV1* NavSys, const FVector& Location) {
    SetActorLocation(Location);

    // ナビメッシュの範囲内かどうか
    auto Pos = Location;
    Pos.Z = 0.f;
    FNavLocation OutStart;
    if (NavSys->ProjectPointToNavigation(Pos, OutStart, FVector::One() * 100) == false) {
        State = TwinLinkNavSystemPathLocatorState::OutsideNavMesh;
        return false;
    }
    OutStart.Location.Z = Location.Z;
    State = TwinLinkNavSystemPathLocatorState::Valid;
    LastValidLocation = OutStart.Location;
    return true;
}

void ATwinLinkNavSystemPathLocator::Select() {
    IsSelected = true;
}

void ATwinLinkNavSystemPathLocator::UnSelect() {
    if (LastValidLocation.has_value()) {
        SetActorLocation(*LastValidLocation);
        State = TwinLinkNavSystemPathLocatorState::Valid;
    }
    //LastValidLocation = std::nullopt;
    IsSelected = false;
}

std::optional<FVector> ATwinLinkNavSystemPathLocator::GetLastValidLocation() const {
    return LastValidLocation;
}

ECollisionChannel ATwinLinkNavSystemPathLocator::GetCollisionChannel() const {
    return ECC_WorldStatic;
}

void ATwinLinkNavSystemPathLocator::BeginDestroy() {
    Super::BeginDestroy();
}

void ATwinLinkNavSystemPathLocator::Destroyed() {
    Super::Destroyed();
    if (WarnTextWidget) {
        auto Parent = WarnTextWidget->GetParent();
        WarnTextWidget->RemoveFromParent();
        //delete WarnTextWidget;
        WarnTextWidget = nullptr;
    }
}
