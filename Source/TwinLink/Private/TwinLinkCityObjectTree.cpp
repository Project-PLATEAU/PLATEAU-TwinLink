#pragma once
#include "TwinLinkCityObjectTree.h"

#include <chrono>

#include "TwinLinkActorEx.h"
#include "TwinLinkMathEx.h"
#include "TwinLinkPLATEAUCityObjectGroupEx.h"
#include "TwinLinkPLATEAUGeoReferenceEx.h"
#include "Kismet/GameplayStatics.h"

FTwinLinkCityObjectQuadTreeKey FTwinLinkCityObjectQuadTreeKey::Create(const FTwinLinkSpatialID& SpId) {
    return FTwinLinkCityObjectQuadTreeKey{ SpId.GetX(), SpId.GetY(), SpId.GetZ() };
}

ATwinLinkCityObjectTree* ATwinLinkCityObjectTree::Instance(const UWorld* World) {
    return TwinLinkActorEx::FindFirstActorInWorld<ATwinLinkCityObjectTree>(World);
}

void ATwinLinkCityObjectTree::Init(APLATEAUInstancedCityModel* CityModel) {
    if (!CityModel)
        return;
    auto& GeoRef = CityModel->GeoReference;
    auto& Data = GeoRef.GetData();
    InstancedCityModel = CityModel;
    FVector Center;
    FVector Extent;
    InstancedCityModel->GetActorBounds(false, Center, Extent, true);
    RangeWorld = FBox(Center - Extent, Center + Extent);

    RangeVoxelSpace = FBox(
        FTwinLinkSpatialID::WorldToVoxelSpace(RangeWorld.Min, GeoRef),
        FTwinLinkSpatialID::WorldToVoxelSpace(RangeWorld.Max, GeoRef)
    );

    QuadTreeMap.Reset();
    // CityModel以下にある全CityObjectGroupを登録する
    auto Scanner = TwinLinkPLATEAUInstancedCityModelScanner(InstancedCityModel);
    for (auto Item : Scanner) {
        if (Item.MeshType != FTwinLinkFindCityModelMeshType::Bldg)
            continue;
        Add(Item.CityObjectGroup);
    }
}

FPLATEAUGeoReference* ATwinLinkCityObjectTree::GetGeoReference() const {
    if (const auto Model = GetInstancedCityModel().Get())
        return &Model->GeoReference;
    return nullptr;
}

void ATwinLinkCityObjectTree::Add(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject) {
    if (CityObject.IsValid() == false)
        return;
    auto& GeoRef = InstancedCityModel->GeoReference;
    if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityObject.Get(), Bb)) {
        FTwinLinkSpatialID SpId;
        if (FTwinLinkSpatialID::TryGetBoundingSpatialId(GeoRef, Bb, false, SpId)) {
            auto Tree = GetOrCreateTree(SpId);
            Tree->BinaryTree.Nodes.Add(CityObject);
        }
        else {
            OutOfRangedTree.Nodes.Add(CityObject);
        }
    }
}

ATwinLinkCityObjectTree::ATwinLinkCityObjectTree() {
    PrimaryActorTick.bCanEverTick = true;
}

void ATwinLinkCityObjectTree::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

#ifdef WITH_EDITOR
    DebugDraw(DeltaSeconds);
#endif
}

TArray<ATwinLinkCityObjectTree::FCityObjectFindInfo> ATwinLinkCityObjectTree::GetCityObjectGroups(
    const FTwinLinkSpatialID& SpatialId) const {

    TArray<FCityObjectFindInfo> Ret;
    const auto SpacialWorldBox = SpatialId.GetSpatialIDArea(InstancedCityModel->GeoReference);
    // 領域の範囲外は無視する
    if (RangeWorld.IsInsideXY(SpacialWorldBox) == false)
        return Ret;

    // 自身と子ツリーから検索する
    FindChild(SpatialId, SpacialWorldBox, Ret);

    for (auto Zoom = SpatialId.GetZ() - 1; Zoom >= FTwinLinkSpatialID::MIN_ZOOM_LEVEL; --Zoom) {
        auto Id = SpatialId.ZoomChanged(Zoom);
        const auto ParentTree = GetTree(Id);
        if (!ParentTree)
            continue;
        for (auto& N : ParentTree->BinaryTree.Nodes) {
            if (IsTarget(N, Zoom, SpacialWorldBox)) {
                Ret.Add(FCityObjectFindInfo(N, Id));
            }
        }
    }
    return Ret;
}

void ATwinLinkCityObjectTree::DebugDraw(float DeltaSeconds) {

#ifdef WITH_EDITOR
    if (DebugShowSpace) {
        auto& GeoRef = InstancedCityModel->GeoReference;
        DebugShowSpaceId.Range.Min = FTwinLinkSpatialID::WorldToVoxelSpace(RangeWorld.Min, GeoRef, DebugShowSpaceId.Z);
        DebugShowSpaceId.Range.Max = FTwinLinkSpatialID::WorldToVoxelSpace(RangeWorld.Max, GeoRef, DebugShowSpaceId.Z);
        DebugShowSpaceId.X = FMath::Clamp(DebugShowSpaceId.X, (int)FMath::CeilToInt(1.f * DebugShowSpaceId.Range.Min.X), (int)FMath::FloorToInt(1.f * DebugShowSpaceId.Range.Max.X));
        DebugShowSpaceId.Y = FMath::Clamp(DebugShowSpaceId.Y, (int)FMath::CeilToInt(1.f * DebugShowSpaceId.Range.Min.Y), (int)FMath::FloorToInt(1.f * DebugShowSpaceId.Range.Max.Y));
        const auto SpatialId = DebugShowSpaceId.ToSpatialID();
        auto Box = SpatialId.GetSpatialIDArea(GeoRef);
        Box.Min.Z = RangeWorld.Min.Z;
        Box.Max.Z = RangeWorld.Max.Z;
        DrawDebugBox(GetWorld(), Box.GetCenter(), Box.GetExtent(), FColor::Red);

        const auto Scanner = TwinLinkPLATEAUInstancedCityModelScanner(InstancedCityModel);
        for (auto Item : Scanner) {
            if (DebugShowCompName.IsEmpty() == false && Item.CityObjectGroup->GetName().StartsWith(DebugShowCompName)) {
                FBox Bb;
                FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(Item.CityObjectGroup.Get(), Bb);

                DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent(), FColor::Blue);

                FTwinLinkSpatialID SpId;
                FTwinLinkSpatialID::TryGetBoundingSpatialId(GeoRef, Bb, false, SpId);
                auto SBb = SpId.GetSpatialIDArea(InstancedCityModel->GeoReference);
                SBb.Min.Z = RangeWorld.Min.Z;
                SBb.Max.Z = RangeWorld.Max.Z;
                DrawDebugBox(GetWorld(), SBb.GetCenter(), SBb.GetExtent(), FColor::Red);
                auto P = Bb.GetCenter();
                P.Z = Bb.Max.Z;
                UKismetSystemLibrary::DrawDebugString(this, P, FString::Printf(TEXT("%d: %d, %d"), SpId.GetZ(), SpId.GetX(), SpId.GetY()));
            }
        }

        // 8分木探索表示
        if (DebugShowSpaceGroup) {
            auto start = std::chrono::system_clock::now(); // 計測開始時間
            auto Objects = GetCityObjectGroups(SpatialId);
            auto end = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("QuadTree %lld"), elapsed), true, false, FLinearColor::Red, 0.f);

            for (auto CityObject : Objects) {
                if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityObject.CityObjectGroup.Get(), Bb))
                    DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent(), FColor::Green);
            }
        }
        // 全探索表示
        if (DebugShowSpaceDebug) {
            auto start = std::chrono::system_clock::now(); // 計測開始時間
            auto Objects = DebugGetCityObjectGroups(SpatialId);
            auto end = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Debug %lld"), elapsed), true, false, FLinearColor::Red, 0.f);

            for (auto CityObject : Objects) {
                if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityObject.Get(), Bb)) {
                    auto Pos = Bb.GetCenter();
                    Pos.Z = Bb.Max.Z;
                    UKismetSystemLibrary::DrawDebugString(this, Pos, CityObject->GetName());
                    DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent() - FVector::One() * 10, FColor::Blue);
                }
            }
        }
    }
#endif
}

ATwinLinkCityObjectTree::KeyType ATwinLinkCityObjectTree::ToKey(const FTwinLinkSpatialID& SpId) {
    return KeyType::Create(SpId);
}

FTwinLinkCityObjectQuadTree* ATwinLinkCityObjectTree::GetOrCreateTree(const FTwinLinkSpatialID& SpId) {
    auto Key = ToKey(SpId);
    // すでに作成済みの場合はそのまま返す
    if (QuadTreeMap.Contains(Key))
        return &QuadTreeMap[Key];

    // 作成していない場合は親ツリーも構築して返す
    QuadTreeMap.Add(Key, FTwinLinkCityObjectQuadTree());
    return &QuadTreeMap[Key];
}

const FTwinLinkCityObjectQuadTree* ATwinLinkCityObjectTree::GetTree(const FTwinLinkSpatialID& SpId) const {
    return GetTree(ToKey(SpId));
}

const FTwinLinkCityObjectQuadTree* ATwinLinkCityObjectTree::GetTree(const FTwinLinkCityObjectQuadTreeKey& Key) const {
    if (QuadTreeMap.Contains(Key) == false)
        return nullptr;
    return &QuadTreeMap[Key];
}

void ATwinLinkCityObjectTree::FindChild(const FTwinLinkSpatialID& SpId, const FBox& SpacialWorldBox,
    TArray<FCityObjectFindInfo>& Out) const {

    // 自分の担当している子ツリーがいるかチェック
    if (const auto Tree = GetTree(SpId)) {
        for (auto& N : Tree->BinaryTree.Nodes) {
            if (IsTarget(N, SpId.GetZ(), SpacialWorldBox)) {
                Out.Add(FCityObjectFindInfo(N, SpId));
            }
        }
    }

    if (SpId.GetZ() >= FTwinLinkSpatialID::MAX_ZOOM_LEVEL)
        return;
    // メモリ最適化の関係上親がいなくても子はいる場合がある(自分の担当ノードがいるツリーしか作成しないため)
    // そのためTreeがない場合でも子は検索する必要がある
    constexpr int Dx[] = { 0, 1, 1, 0 };
    constexpr int Dy[] = { 0, 0, 1, 1 };
    const auto ChildIdNw = SpId.ZoomChanged(SpId.GetZ() + 1);
    for (auto I = 0; I < 4; ++I) {
        const auto Id = FTwinLinkSpatialID::Create(ChildIdNw.GetZ(), 0, ChildIdNw.GetX() + Dx[I], ChildIdNw.GetY() + Dy[I]);
        FindChild(Id, SpacialWorldBox, Out);
    }
}

bool ATwinLinkCityObjectTree::IsTarget(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject, int Zoom,
    const FBox& SpatialWorldBox) {
    if (CityObject.IsValid() == false)
        return false;
    if (CityObject->GetVisibleFlag() == false)
        return false;
    // 最大ズームレベルだと確定でOK
    if (Zoom >= FTwinLinkSpatialID::MAX_ZOOM_LEVEL)
        return true;
    if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityObject.Get(), Bb)) {
        // #TODO : 場合によってはどこにも属さない建物が出てくるのでかなり低めに設定している
        constexpr float AREA_RATE = 0.1f;
        const auto IntersectArea = TwinLinkMathEx::GetIntersectBoxXY(Bb, SpatialWorldBox).GetArea();
        const auto BbArea = TwinLinkMathEx::GetAreaXY(Bb);
        const auto SpatialArea = TwinLinkMathEx::GetAreaXY(SpatialWorldBox);
        return IntersectArea > BbArea * AREA_RATE || IntersectArea > SpatialArea * AREA_RATE;
    }
    return false;
}

TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> ATwinLinkCityObjectTree::DebugGetCityObjectGroups(
    const FTwinLinkSpatialID& SpatialId) const {
    TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> Ret;
    const auto Box = SpatialId.GetSpatialIDArea(InstancedCityModel->GeoReference);

    if (RangeWorld.IsInsideXY(Box) == false)
        return Ret;
    const auto Scanner = TwinLinkPLATEAUInstancedCityModelScanner(InstancedCityModel);
    for (const auto Item : Scanner) {
        if (Item.MeshType != FTwinLinkFindCityModelMeshType::Bldg)
            continue;
        if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(Item.CityObjectGroup.Get(), Bb)) {
            if (Item.CityObjectGroup.IsValid() == false)
                continue;
            if (Item.CityObjectGroup->GetVisibleFlag() == false)
                continue;
            if (Bb.IntersectXY(Box)) {
                Ret.Add(Item.CityObjectGroup);
            }
        }
    }
    return Ret;
}
