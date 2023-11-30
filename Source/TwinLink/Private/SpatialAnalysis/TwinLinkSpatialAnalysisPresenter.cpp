// Copyright (C) 2023, MLIT Japan. All rights reserved.
#include "SpatialAnalysis/TwinLinkSpatialAnalysisPresenter.h"

#include "TwinLinkCityObjectTree.h"
#include "TwinLinkCommon.h"
#include "Misc/TwinLinkPLATEAUCityObjectGroupEx.h"
#include "TwinLinkPOISubSystem.h"
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

    // 人流データ取得API構築
    PeopleFlowApi = NewObject<UTwinLinkPeopleFlowApi>();
    PeopleFlowApi->OnReceivedPeopleFlowResponse.AddDynamic(this, &ATwinLinkSpatialAnalysisPresenter::OnReceivedPeopleFlowResponse);

    // Emissiveが差し替えられるようにDynamicMaterialに
    if (const auto Mesh = GetComponentByClass<UStaticMeshComponent>()) {
        const auto BaseMat = Mesh->GetMaterial(0);
        DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMat);

        const auto bIsNight = TwinLinkGraphicsEnv::GetNightIntensity(GetWorld());
        DynamicMaterial->SetScalarParameterValue(FName(TEXT("IsNight")), bIsNight ? 1.f : 0.f);
    }
}

void ATwinLinkSpatialAnalysisPresenter::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    DrawUpdate(DeltaTime);
#if WITH_EDITOR
    DebugNowSelectedId = GetNowSpatialId().value_or(FTwinLinkSpatialID());
    DebugDrawUpdate(DeltaTime);
#endif
}

void ATwinLinkSpatialAnalysisPresenter::CheckSpatialIdChanged(const std::optional<FTwinLinkSpatialID>& Before) const {
    const auto Tree = GetTree();
    if (!Tree)
        return;
    const auto After = GetNowSpatialId();
    if (After.has_value() == false)
        return;

    if (!Before.has_value() || *Before != *After) {
        const auto POISystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
        check(POISystem.IsValid());
        const auto Bb = After->GetSpatialIDArea(*Tree->GetGeoReference());
        TMap<FString, TMap<FString, FVector>> PoiMap;

        const auto Buildings = Tree->FindCityObjectGroups(*After);
        FTwinLinkSpatialAnalysisUiInfo Info;
        Info.SpatialId = *After;
        Info.BuildingCount = Buildings.Num();

        // 各登録名ごとの数を入れる
        POISystem->TwinLinkPOIGetInsidePOI(Bb, PoiMap);
        for (auto& M : PoiMap) {
            Info.PoiInfos.Add(FTwinLinkSpatialAnalysisPoiInfo{ M.Key, M.Value.Num() });
        }

        OnSpatialIdChanged.Broadcast(*Before, Info);
    }
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
    const auto Before = GetNowSpatialId();
    this->Zoom = Value;
    CheckSpatialIdChanged(Before);
}

const ATwinLinkCityObjectTree* ATwinLinkSpatialAnalysisPresenter::GetTree() const {
    return CacheTree.Get();
}

void ATwinLinkSpatialAnalysisPresenter::OnClicked(const FHitResult& HitResult) {
    if (const auto Tree = GetTree()) {
        if (Tree->GetRangeWorld().IsInsideXY(HitResult.Location) == false)
            return;
        const auto Before = GetNowSpatialId();
        NowHitResult = HitResult;
        CheckSpatialIdChanged(Before);
    }
}

void ATwinLinkSpatialAnalysisPresenter::DrawUpdate(float DeltaTime) const {
    auto Bb = [&]()-> std::optional<FBox> {
        if (IsValidSpatialId() == false)
            return std::nullopt;

        const auto NavSystem = ATwinLinkNavSystem::GetInstance(GetWorld());
        if (!NavSystem)
            return std::nullopt;

        const auto NowSelectedId = GetNowSpatialId();
        if (NowSelectedId.has_value() == false)
            return std::nullopt;
        const auto Tree = GetTree();
        const auto DemBb = NavSystem->GetDemCollisionAabb();
        const auto RangeWorld = Tree->GetRangeWorld();
        auto Bb = NowSelectedId->GetSpatialIDArea(*Tree->GetGeoReference());
        auto Min = Bb.Min;
        Min.Z = DemBb.Min.Z;
        auto Max = Bb.Max;
        Max.Z = RangeWorld.Max.Z;
        return FBox(Min, Max);
    }();
   

    if(const auto Mesh = GetComponentByClass<UStaticMeshComponent>())
    {
        Mesh->SetVisibility(Bb.has_value());
        Mesh->SetWorldLocation(Bb->GetCenter());
        Mesh->SetWorldScale3D(Bb->GetSize() * 0.01f);
    }
}

void ATwinLinkSpatialAnalysisPresenter::DebugDrawUpdate(float DeltaTime) const {
#if WITH_EDITOR
    if (!DebugShowSpace)
        return;
    if (IsValidSpatialId() == false)
        return;
    const auto Tree = GetTree();
    if (!Tree)
        return;
    const auto Buildings = Tree->FindCityObjectGroups(*GetNowSpatialId());
    for (auto CityObject : Buildings) {
        if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityObject.CityObjectGroup.Get(), Bb))
            DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent(), FColor::Green);
    }
#endif
}

void ATwinLinkSpatialAnalysisPresenter::OnReceivedPeopleFlowResponse(const FTWinLinkPeopleFlowApiResult& Result) {
    if (!Result.bSuccess)
        return;
    const auto NowSpacialId = GetNowSpatialId();
    if (NowSpacialId.has_value() == false)
        return;
    for (auto& P : Result.Populations) {
        if (P.SpatialId.EqualZXY(*NowSpacialId)) {
            OnChangePeopleFlow.Broadcast(P);
            break;
        }
    }
}

bool ATwinLinkSpatialAnalysisPresenter::TryGetNowSpatialId(FTwinLinkSpatialID& Out) const {
    auto Ret = GetNowSpatialId();
    if (Ret.has_value() == false)
        return false;
    Out = *Ret;
    return true;
}

bool ATwinLinkSpatialAnalysisPresenter::RequestPeopleFlow(FDateTime DateTime) {
    const auto SpacialId = GetNowSpatialId();
    if (SpacialId.has_value() == false)
        return false;
    if (PeopleFlowApi) {
        FTwinLinkPeopleFlowApiRequest Req;
        Req.SpatialIds.Add(*SpacialId);
        Req.DateTime = DateTime;
        PeopleFlowApi->Request(Req);
        return true;
    }
    return false;
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