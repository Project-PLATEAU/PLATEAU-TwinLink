// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "NavSystem/TwinLinkNavSystemPathLocator.h"

#include "NavigationSystem.h"
#include "TwinLinkMathEx.h"
#include "TwinLinkWorldViewer.h"
#include "NavSystem/TwinLinkNavSystem.h"

// Sets default values
ATwinLinkNavSystemPathLocator::ATwinLinkNavSystemPathLocator() {
    PrimaryActorTick.bCanEverTick = true;
}

void ATwinLinkNavSystemPathLocator::BeginPlay() {
    Super::BeginPlay();
}

void ATwinLinkNavSystemPathLocator::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    if (auto WorldViewer = ATwinLinkNavSystem::GetWorldViewer(GetWorld())) {
        const auto Forward = WorldViewer->GetNowCameraRotationOrDefault().RotateVector(FVector::ForwardVector);
        // カメラのForwardと反対方向を向かせる
        // メッシュがY軸Forwardだったので合わせる
        const auto Rotation = FRotationMatrix::MakeFromYZ(-Forward, FVector::UpVector).ToQuat();
        SetActorRotation(Rotation);
    }
}

NavSystemPathLocatorState ATwinLinkNavSystemPathLocator::GetNowState() const {
    return State;
}

bool ATwinLinkNavSystemPathLocator::UpdateLocation(const UNavigationSystemV1* NavSys, const FHitResult& HitResult) {
    SetActorLocation(HitResult.Location);

    // 壁についている
    if (FVector::DotProduct(HitResult.Normal, FVector::UpVector) < FMath::Cos(FMath::DegreesToRadians(70))) {
        State = NavSystemPathLocatorState::OnWall;
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
        State = NavSystemPathLocatorState::OutsideNavMesh;
        return false;
    }
    OutStart.Location.Z = Location.Z;
    State = NavSystemPathLocatorState::Valid;
    LastValidLocation = OutStart.Location;
    return true;
}

void ATwinLinkNavSystemPathLocator::Select() {
    IsSelected = true;
}

void ATwinLinkNavSystemPathLocator::UnSelect() {
    if (LastValidLocation.has_value()) {
        SetActorLocation(*LastValidLocation);
        State = NavSystemPathLocatorState::Valid;
    }
    //LastValidLocation = std::nullopt;
    IsSelected = false;
}

std::optional<FVector> ATwinLinkNavSystemPathLocator::GetLastValidLocation() const {
    return LastValidLocation;
}