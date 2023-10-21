// Copyright (C) 2023, MLIT Japan. All rights reserved.
#include "SpatialAnalysis/TwinLinkSpatialAnalysisPresenter.h"

#include "TwinLinkCityObjectTree.h"
#include "TwinLinkWorldViewer.h"
#include "NavSystem/TwinLinkNavSystem.h"

ATwinLinkSpatialAnalysisPresenter::ATwinLinkSpatialAnalysisPresenter() {
    PrimaryActorTick.bCanEverTick = true;
}

void ATwinLinkSpatialAnalysisPresenter::BeginPlay() {
    Super::BeginPlay();
    CacheTree = ATwinLinkCityObjectTree::Instance(GetWorld());

    // クリック時のコールバック登録
    if (const auto WorldViewer = ATwinLinkNavSystem::GetWorldViewer(GetWorld()))
        WorldViewer->EvOnAnyObjectClicked.AddUObject(this, &ATwinLinkSpatialAnalysisPresenter::OnClicked);
    SetZoom(FTwinLinkSpatialID::MAX_ZOOM_LEVEL);
}

void ATwinLinkSpatialAnalysisPresenter::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    DrawUpdate(DeltaTime);

    DebugNowSelectedId = GetNowSpatialId().value_or(FTwinLinkSpatialID());
}

bool ATwinLinkSpatialAnalysisPresenter::IsValidSpatialId() const {
    auto NowSelectedId = GetNowSpatialId();
    if (NowSelectedId.has_value() == false)
        return false;
    if (const auto Tree = GetTree()) {
        return Tree->GetRangeVoxelSpace().IsInsideXY((*NowSelectedId).ZoomChanged(FTwinLinkSpatialID::MAX_ZOOM_LEVEL).ToVector());
    }
    return false;
}

void ATwinLinkSpatialAnalysisPresenter::SetZoom(int Value) {
    this->Zoom = Value;
}

const ATwinLinkCityObjectTree* ATwinLinkSpatialAnalysisPresenter::GetTree() const {
    return CacheTree.Get();
}

void ATwinLinkSpatialAnalysisPresenter::OnClicked(const FHitResult& Info) {
    if (const auto Tree = GetTree()) {
        if (Tree->GetRangeWorld().IsInsideXY(Info.Location) == false)
            return;
        const auto Before = GetNowSpatialId();
        NowHitResult = Info;
        const auto After = GetNowSpatialId();
        if (!Before.has_value() || *Before != *After)
            OnSpatialIdChanged.Broadcast(*Before, *After);
    }
}

void ATwinLinkSpatialAnalysisPresenter::DrawUpdate(float DeltaTime) {
    if (IsValidSpatialId() == false)
        return;

    const auto NavSystem = ATwinLinkNavSystem::GetInstance(GetWorld());
    if (!NavSystem)
        return;

    auto NowSelectedId = GetNowSpatialId();
    if (NowSelectedId.has_value() == false)
        return;
    const auto Tree = GetTree();
    const auto DemBb = NavSystem->GetDemCollisionAabb();
    const auto RangeWorld = Tree->GetRangeWorld();
    auto Bb = NowSelectedId->GetSpatialIDArea(*Tree->GetGeoReference());
    auto Min = Bb.Min;
    Min.Z = DemBb.Min.Z;
    auto Max = Bb.Max;
    Max.Z = RangeWorld.Max.Z;
    Bb = FBox(Min, Max);
    DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent(), FColor::Red);
}

bool ATwinLinkSpatialAnalysisPresenter::TryGetNowSpatialId(FTwinLinkSpatialID& Out) const {
    auto Ret = GetNowSpatialId();
    if (Ret.has_value() == false)
        return false;
    Out = *Ret;
    return true;
}

std::optional<FTwinLinkSpatialID> ATwinLinkSpatialAnalysisPresenter::GetNowSpatialId() const {
    if (NowHitResult.IsValidBlockingHit() == false)
        return std::nullopt;

    if (const auto Tree = GetTree()) {
        if (Tree->GetRangeWorld().IsInsideXY(NowHitResult.Location) == false)
            return std::nullopt;
        return FTwinLinkSpatialID::Create(*Tree->GetGeoReference(), NowHitResult.Location, Zoom, false);
    }
    return std::nullopt;
}
