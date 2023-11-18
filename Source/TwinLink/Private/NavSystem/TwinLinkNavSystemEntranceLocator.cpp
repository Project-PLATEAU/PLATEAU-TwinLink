// Copyright (C) 2023, MLIT Japan. All rights reserved.
#include "NavSystem/TwinLinkNavSystemEntranceLocator.h"
#include <optional>
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include <NavSystem/TwinLinkNavSystemPathLocator.h>
#include "TwinLinkPlayerController.h"
#include "TwinLinkWidgetEx.h"
#include "Components/PanelWidget.h"
#include "NavSystem/TwinLinkNavSystem.h"

TArray<FTwinLinkEntranceLocatorNode*>& FTwinLinkEntranceLocatorNode::AllNodes() {
    static TArray<FTwinLinkEntranceLocatorNode*> Ret;
    return Ret;
}

bool FTwinLinkEntranceLocatorNode::IsAnyNodeVisible() {
    for (const auto& N : AllNodes()) {
        if (N->IsVisibleEntranceLocator())
            return true;
    }
    return false;
}

FTwinLinkEntranceLocatorNode::FTwinLinkEntranceLocatorNode() {
    AllNodes().Add(this);
}

FTwinLinkEntranceLocatorNode::~FTwinLinkEntranceLocatorNode() {
    AllNodes().Remove(this);
}

std::optional<FVector> FTwinLinkEntranceLocatorNode::GetEntranceLocation() const {
    if (const auto Locator = ATwinLinkNavSystem::GetEntranceLocator(GetWorld()))
        return Locator->GetPathLocation();
    return std::nullopt;
}

bool FTwinLinkEntranceLocatorNode::SetDefaultEntranceLocation(const FString& FeatureId) const {
    // 見える設定じゃないなら無視する
    if (IsVisibleEntranceLocator() == false)
        return false;
    if (const auto Locator = ATwinLinkNavSystem::GetEntranceLocator(GetWorld())) {
        Locator->SetDefaultEntranceLocation(FeatureId);
        return true;
    }
    return false;
}

bool FTwinLinkEntranceLocatorNode::SetEntranceLocation(const UTwinLinkFacilityInfo* Info) const {
    // 見える設定じゃないなら無視する
    if (IsVisibleEntranceLocator() == false)
        return false;
    if (const auto Locator = ATwinLinkNavSystem::GetEntranceLocator(GetWorld())) {
        Locator->SetEntranceLocation(Info);
        return true;
    }
    return false;
}


FTwinLinkEntranceLocatorWidgetNode::FTwinLinkEntranceLocatorWidgetNode(UWidget* W)
    :Widget(W) {
}

bool FTwinLinkEntranceLocatorWidgetNode::IsVisibleEntranceLocator() const {
    if (Widget.IsValid() == false)
        return false;
    return TwinLinkWidgetEx::IsVisibleIncludeOuter(Widget.Get());
}

const UWorld* FTwinLinkEntranceLocatorWidgetNode::GetWorld() const {
    if (Widget.IsValid())
        return Widget->GetWorld();
    return nullptr;
}

ATwinLinkNavSystemEntranceLocator::ATwinLinkNavSystemEntranceLocator() {
    PrimaryActorTick.bCanEverTick = true;
}

ATwinLinkNavSystemEntranceLocator::~ATwinLinkNavSystemEntranceLocator() {
}

void ATwinLinkNavSystemEntranceLocator::BeginPlay() {
    Super::BeginPlay();

    // #TODO : マウス管理を各アクターで行うべきではない
    APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    // 入力を有効にする
    EnableInput(controller);
    // マウスカーソルオンにする
    controller->SetShowMouseCursor(true);
}

// Called every frame
void ATwinLinkNavSystemEntranceLocator::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    const auto TwinLinkNavSystem = ATwinLinkNavSystem::GetInstance(GetWorld());
    const auto DemCollisionAabb = TwinLinkNavSystem->GetDemCollisionAabb();

    // 
    const APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PlayerController)
        return;
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return;
    // 押したとき
    auto MousePos = ATwinLinkPlayerController::GetMousePosition(PlayerController);
    if (MousePos.has_value() == false)
        return;
    if (PlayerController->WasInputKeyJustPressed(EKeys::LeftMouseButton)) {
        auto Ray = ATwinLinkPlayerController::ScreenToWorldRayThroughBoundingBox(PlayerController, *MousePos, DemCollisionAabb, 100);
        if (Ray.has_value()) {
            TArray<FHitResult> HitResults;
            FCollisionQueryParams Params;
            if (GetWorld()->LineTraceMultiByChannel(HitResults, Ray->Min, Ray->Max, ECC_WorldStatic)) {
                auto bPickMe = false;
                for (auto& HitResult : HitResults) {
                    bPickMe = HitResult.GetActor() == this;
                    if (bPickMe) {
                        NowSelectedPathLocatorActorScreenOffset = FVector2D::Zero();
                        FVector2D ScreenPos = FVector2D::Zero();
                        if (UGameplayStatics::ProjectWorldToScreen(PlayerController, GetActorLocation(), ScreenPos))
                            NowSelectedPathLocatorActorScreenOffset = ScreenPos - *MousePos;
                        break;
                    }
                }
                if (bPickMe) {
                    Select();
                }
            }
        }
    }
    // 離した時
    else if (PlayerController->WasInputKeyJustReleased(EKeys::LeftMouseButton)) {
        if (IsSelected) {
            UnSelect();
        }
    }
    // ドラッグ中
    else if (PlayerController->IsInputKeyDown(EKeys::LeftMouseButton)) {
        if (IsSelected) {
            const auto ScreenPos = *MousePos + NowSelectedPathLocatorActorScreenOffset;
            const auto Ray = ATwinLinkPlayerController::ScreenToWorldRayThroughBoundingBox(PlayerController, ScreenPos, DemCollisionAabb, 100);
            if (Ray.has_value()) {
                TArray<FHitResult> HitResults;
                if (GetWorld()->LineTraceMultiByChannel(HitResults, Ray->Min, Ray->Max, ECollisionChannel::ECC_WorldStatic)) {
                    for (auto& HitResult : HitResults) {
                        if (HitResult.IsValidBlockingHit() == false)
                            continue;

                        UpdateLocation(NavSys, HitResult);
                        break;
                    }
                }
            }
        }
    }
}

bool ATwinLinkNavSystemEntranceLocator::TryGetLocation(FVector& Out) const {
    auto Ret = GetPathLocation();
    if (Ret.has_value() == false)
        return false;
    Out = *Ret;
    return true;
}

std::optional<FVector> ATwinLinkNavSystemEntranceLocator::GetPathLocation() const {
    return GetLastValidLocation();
}

void ATwinLinkNavSystemEntranceLocator::SetEntranceLocation(const UTwinLinkFacilityInfo* Info) {
    if (Info->GetEntrances().Num() > 0) {
        UpdateLocation(FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()), Info->GetEntrances()[0]);
    }
    // 設定されていない場合は建物のBbからデフォルト位置を設定する
    else {
        SetDefaultEntranceLocation(Info->GetFeatureID());
    }
}

void ATwinLinkNavSystemEntranceLocator::SetDefaultEntranceLocation(const FString& FeatureId) {
    FVector Location;
    if (const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld())) {
        if (const auto FacilitySystem = ATwinLinkNavSystem::GetFacilityInfoSystem(GetWorld())) {
            const auto Facility = FacilitySystem->FindFacility(FeatureId);
            if (Facility.IsValid()) {
                FVector OutPos;
                if (ATwinLinkNavSystem::GetInstance(GetWorld())->FindNavMeshPoint(NavSys, Cast<UStaticMeshComponent>(Facility), OutPos))
                    Location = OutPos;
                else
                    Location = Facility->GetComponentLocation();
            }
        }
    }
    UpdateLocation(FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()), Location);
}

bool ATwinLinkNavSystemEntranceLocator::IsValidLocation() const {
    return GetPathLocation().has_value();
}
