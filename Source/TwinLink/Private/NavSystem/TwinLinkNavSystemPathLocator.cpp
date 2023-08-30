// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "NavSystem/TwinLinkNavSystemPathLocator.h"

#include "NavigationSystem.h"

// Sets default values
ATwinLinkNavSystemPathLocator::ATwinLinkNavSystemPathLocator() {
    PrimaryActorTick.bCanEverTick = true;
}

void ATwinLinkNavSystemPathLocator::BeginPlay() {
    Super::BeginPlay();
}

void ATwinLinkNavSystemPathLocator::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
}

void ATwinLinkNavSystemPathLocator::UpdateLocation(const UNavigationSystemV1* NavSys, const FHitResult& HitResult) {
    SetActorLocation(HitResult.Location);

    // 壁についている
    if(FVector::DotProduct(HitResult.Normal,FVector::UpVector) < FMath::Cos(FMath::DegreesToRadians(70)))
    {
        State = NavSystemPathLocatorState::OnWall;
        return;
    }

    // ナビメッシュの範囲内かどうか
    auto Pos = HitResult.Location;
    Pos.Z = 0.f;
    FNavLocation OutStart;
    if (NavSys->ProjectPointToNavigation(Pos, OutStart, FVector::One() * 100) == false) {
        State = NavSystemPathLocatorState::OutsideNavMesh;
        return;
    }
    OutStart.Location.Z = HitResult.Location.Z;
    State = NavSystemPathLocatorState::Valid;
    LastValidLocation = OutStart.Location;
}

void ATwinLinkNavSystemPathLocator::Select()
{
    IsSelected = true;
}

void ATwinLinkNavSystemPathLocator::UnSelect()
{
    if (LastValidLocation.has_value()) {
        SetActorLocation(*LastValidLocation);
        State = NavSystemPathLocatorState::Valid;
    }
    LastValidLocation = std::nullopt;
    IsSelected = false;
}

