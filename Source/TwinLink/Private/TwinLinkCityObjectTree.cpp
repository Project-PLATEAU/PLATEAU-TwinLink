#pragma once
#include "TwinLinkCityObjectTree.h"

#include <chrono>

#include "Misc/TwinLinkActorEx.h"
#include "Misc/TwinLinkMathEx.h"
#include "Misc/TwinLinkPLATEAUCityObjectGroupEx.h"
#include "Misc/TwinLinkPLATEAUGeoReferenceEx.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/TwinLinkPLATEAUCityModelEx.h"

namespace {
}

FTwinLinkCityObjectQuadTreeKey FTwinLinkCityObjectQuadTreeKey::Create(const FTwinLinkSpatialID& SpId) {
    return FTwinLinkCityObjectQuadTreeKey(SpId.GetZ(), SpId.GetX(), SpId.GetY());
}

uint64 FTwinLinkCityObjectQuadTreeKey::AsUInt64() const {
    constexpr auto SHIFT_BIT = 29;
    constexpr auto ZOOM_SHIFT_BIT = (64 - 2 * SHIFT_BIT);
    const auto Key = ZoomChanged(FTwinLinkSpatialID::MAX_ZOOM_LEVEL);
    const auto MortonNo = TwinLinkMathEx::Cell2MotionNo(static_cast<uint32>(Key.X), static_cast<uint32>(Key.Y));
    return (MortonNo << ZOOM_SHIFT_BIT) | static_cast<uint64>(Zoom);
}

bool FTwinLinkCityObjectQuadTreeKey::IsParentOf(const FTwinLinkCityObjectQuadTreeKey& Child) const {
    // 相手の方が大きい空間だと違う
    if (Zoom >= Child.Zoom)
        return false;
    // 同じズームレベルにした時に一致する
    return (*this) == Child.ZoomChanged(Zoom);
}

FTwinLinkCityObjectQuadTreeKey FTwinLinkCityObjectQuadTreeKey::ZoomChanged(int NextZoom) const {
    const auto DZoom = NextZoom - Zoom;
    if (DZoom == 0)
        return *this;

    const auto Bit = FMath::Abs(DZoom);
    if (DZoom < 0) {
        return FTwinLinkCityObjectQuadTreeKey(NextZoom, X >> Bit, Y >> Bit);
    }
    else {
        return FTwinLinkCityObjectQuadTreeKey(NextZoom, X << Bit, Y << Bit);
    }
}

FString FTwinLinkCityObjectQuadTreeKey::StringZXY() const {
    return FString::Printf(TEXT("%d/%d/%d"), Zoom, X, Y);
}

ATwinLinkCityObjectTree* ATwinLinkCityObjectTree::Instance(const UWorld* World) {
    return TwinLinkActorEx::FindFirstActorInWorld<ATwinLinkCityObjectTree>(World);
}

void ATwinLinkCityObjectTree::Init(APLATEAUInstancedCityModel* BaseCityModel) {
    if (!BaseCityModel)
        return;

    InstancedCityModel = BaseCityModel;
    FVector Center;
    FVector Extent;
    InstancedCityModel->GetActorBounds(false, Center, Extent, true);
    RangeWorld = FBox(Center - Extent, Center + Extent);

    auto& GeoRef = BaseCityModel->GeoReference;
    RangeVoxelSpace = FBox(
        FTwinLinkSpatialID::WorldToVoxelSpace(RangeWorld.Min, GeoRef, MAX_ZOOM_LEVEL),
        FTwinLinkSpatialID::WorldToVoxelSpace(RangeWorld.Max, GeoRef, MAX_ZOOM_LEVEL)
    );

    const auto SpId = FTwinLinkSpatialID::GetBoundingSpatialId(GeoRef, RangeWorld, false);
    if (SpId.has_value()) {
        EntireSpaceKey = ToKey(*SpId);
        OffsetKey = EntireSpaceKey.ZoomChanged(MAX_ZOOM_LEVEL);
    }


    using ArrayType = TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>>;
    TMap<FTwinLinkCityObjectQuadTreeKey, ArrayType> Map;

    auto Visit = [&](APLATEAUInstancedCityModel* CityModel) {
        for (auto Item : TwinLinkPLATEAUInstancedCityModelScanner(CityModel)) {
            if (Item.MeshType != FTwinLinkFindCityModelMeshType::Bldg)
                continue;

            if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(Item.CityObjectGroup.Get(), Bb)) {
                auto SpId2 = FTwinLinkSpatialID::GetBoundingSpatialId(GeoRef, Bb, false);
                if (SpId2.has_value() == false)
                    continue;

                auto Key = ToKey(*SpId2);
                if (Map.Contains(Key) == false) {
                    Map.Add(Key, ArrayType());
                }

                auto& Arr = Map[Key];
                Arr.Add(Item.CityObjectGroup);
            }
        }
    };


    // 管理対象建物も入れるために、配置されている全InstancedCityModelをとってくる
    {
        TArray<AActor*> AllCityModel;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APLATEAUInstancedCityModel::StaticClass(), AllCityModel);
        for (auto Actor : AllCityModel) {
            const auto Model = Cast<APLATEAUInstancedCityModel>(Actor);
            if (!Model)
                continue;
            Visit(Model);
        }
    }
    TreeMap.Reset();

    for (const auto& Item : Map) {
        TreeMap.Add(FTwinLinkCityObjectQuadTreeKeyMap{ Item.Key, Item.Value, 0 });
    }

    TreeMap.Sort([](const FTwinLinkCityObjectQuadTreeKeyMap& A, const FTwinLinkCityObjectQuadTreeKeyMap& B) { return A.Key.AsUInt64() < B.Key.AsUInt64(); });
    for (auto I = 0; I < TreeMap.Num();) {
        I = BuildIndexCache(I);
    }
}

FPLATEAUGeoReference* ATwinLinkCityObjectTree::GetGeoReference() const {
    if (const auto Model = GetInstancedCityModel().Get())
        return &Model->GeoReference;
    return nullptr;
}

int ATwinLinkCityObjectTree::BuildIndexCache(int I) {
    auto& N = TreeMap[I];
    const auto StartIndex = I;
    I = I + 1;
    // 次のオブジェクトが自分の子じゃない時はそこで打ち切り
    while (I < TreeMap.Num() && N.Key.IsParentOf(TreeMap[I].Key))
        I = BuildIndexCache(I);
    N.SpaceSize = I - StartIndex;
    return I;
}

ATwinLinkCityObjectTree::ATwinLinkCityObjectTree() {
    PrimaryActorTick.bCanEverTick = true;
    DebugShowSpaceId.Z = 16;
    DebugShowSpaceId.X = 58198;
    DebugShowSpaceId.Y = 25804;
}

void ATwinLinkCityObjectTree::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

#if WITH_EDITOR
    DebugDraw(DeltaSeconds);
#endif
}

void ATwinLinkCityObjectTree::DebugDraw(float DeltaSeconds) {

#if WITH_EDITOR
    if (DebugCheckAllVoxel) {
        DebugCheckAllVoxel = false;
        auto Test = [this](const FTwinLinkSpatialID& SpId) {
            auto DebugBuilding = DebugGetCityObjectGroups(SpId);
            auto TreeBuilding = FindCityObjectGroups(SpId);
            DebugBuilding.Sort([](const FCityObjectFindInfo& A, const FCityObjectFindInfo& B) { return A.CityObjectGroup.Get() < B.CityObjectGroup.Get(); });
            TreeBuilding.Sort([](const FCityObjectFindInfo& A, const FCityObjectFindInfo& B) { return A.CityObjectGroup.Get() < B.CityObjectGroup.Get(); });
            check(DebugBuilding.Num() == TreeBuilding.Num());
            if (DebugBuilding.Num() == TreeBuilding.Num()) {
                for (auto I = 0; I < DebugBuilding.Num(); ++I)
                    check(DebugBuilding[I].CityObjectGroup == TreeBuilding[I].CityObjectGroup);
            }
        };
        for (auto Z = EntireSpaceKey.Zoom; Z <= MAX_ZOOM_LEVEL; ++Z) {
            auto Range = ToKey(FTwinLinkSpatialID::Create(EntireSpaceKey.Zoom, 0, EntireSpaceKey.X + 1, EntireSpaceKey.Y + 1, false).ZoomChanged(Z));
            for (auto X = 0; X < Range.X; ++X) {
                for (auto Y = 0; Y < Range.Y; ++Y) {
                    Test(ToSpatialId(KeyType(Z, X, Y)));
                }
            }
        }

    }
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

        for (auto& Trr : TreeMap) {
            for (auto Item : Trr.Nodes) {
                if (DebugShowCompName.IsEmpty() == false && Item->GetName().StartsWith(DebugShowCompName)) {
                    FBox Bb;
                    FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(Item.Get(), Bb);

                    DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent(), FColor::Blue);

                    FTwinLinkSpatialID SpId;
                    FTwinLinkSpatialID::TryGetBoundingSpatialId(GeoRef, Bb, false, SpId);
                    auto SBb = SpId.GetSpatialIDArea(InstancedCityModel->GeoReference);
                    SBb.Min.Z = RangeWorld.Min.Z;
                    SBb.Max.Z = RangeWorld.Max.Z;
                    //DrawDebugBox(GetWorld(), SBb.GetCenter(), SBb.GetExtent(), FColor::Red);
                    auto P = Bb.GetCenter();
                    P.Z = Bb.Max.Z;
                    UKismetSystemLibrary::DrawDebugString(this, P, FString::Printf(TEXT("%d: %d, %d"), SpId.GetZ(), SpId.GetX(), SpId.GetY()));
                }
            }
        }


        // 8分木探索表示
        TArray<FCityObjectFindInfo> TreeFindResult;
        TArray<FCityObjectFindInfo> DebugFindResult;

        if (DebugShowSpaceGroup) {
            auto start = std::chrono::system_clock::now(); // 計測開始時間
            TreeFindResult = FindCityObjectGroups(SpatialId);
            auto end = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("QuadTree2 %lld"), elapsed), true, false, FLinearColor::Red, 0.f);

            for (auto CityObject : TreeFindResult) {
                if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityObject.CityObjectGroup.Get(), Bb))
                    DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent(), FColor::Yellow);
            }
        }

        // 全探索表示
        if (DebugShowSpaceDebug) {
            auto start = std::chrono::system_clock::now(); // 計測開始時間
            DebugFindResult = DebugGetCityObjectGroups(SpatialId);
            auto end = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Debug %lld"), elapsed), true, false, FLinearColor::Red, 0.f);

            for (auto CityObject : DebugFindResult) {
                if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityObject.CityObjectGroup.Get(), Bb)) {
                    auto Pos = Bb.GetCenter();
                    Pos.Z = Bb.Max.Z;
                    if (DebugShowSpaceDebugName)
                        UKismetSystemLibrary::DrawDebugString(this, Pos, CityObject.CityObjectGroup->GetName());
                    DrawDebugBox(GetWorld(), Bb.GetCenter(), Bb.GetExtent() - FVector::One() * 10, FColor::Blue);
                }
            }
        }

        if (DebugShowSpaceDebug && DebugShowSpaceGroup) {
            for (auto& F : DebugFindResult) {
                auto A = TreeFindResult.FindByPredicate([&](const FCityObjectFindInfo& Info) { return Info.CityObjectGroup == F.CityObjectGroup; });
                if (A == nullptr) {
                    auto SpKey = ToKey(SpatialId);
                    auto Bb = FTwinLinkPLATEAUCityObjectGroupEx::GetBoundingBox(F.CityObjectGroup.Get());
                    auto Id = ToKey(*FTwinLinkSpatialID::GetBoundingSpatialId(*GetGeoReference(), *Bb, false));
                    auto PId = ToKey(SpatialId.ZoomChanged(Id.Zoom));
                    auto CId = Id.ZoomChanged(SpatialId.GetZ());

                    auto DebugKey = DebugFindKey(F.CityObjectGroup);
                    auto Extent = Bb->GetExtent();
                    Extent.Z *= 100;

                    DrawDebugBox(GetWorld(), Bb->GetCenter(), Extent, FColor::Purple);
                    auto Msg = FString::Printf(TEXT("%s:Sp[%s-%s][%s-%s][%s]"), *(F.CityObjectGroup->GetName()), *SpKey.StringZXY(), *PId.StringZXY(), *Id.StringZXY(), *CId.StringZXY(), *DebugKey->StringZXY());
                    UKismetSystemLibrary::PrintString(this, Msg, true, false, FLinearColor::Red, 0.f);
                }
            }
        }
    }
#endif
}

std::optional<ATwinLinkCityObjectTree::KeyType> ATwinLinkCityObjectTree::DebugFindKey(
    TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup) const {
    for (auto I = 0; I < TreeMap.Num(); ++I) {
        if (TreeMap[I].Nodes.Contains(CityObjectGroup))
            return TreeMap[I].Key;
    }
    return std::nullopt;
}

TArray<ATwinLinkCityObjectTree::FCityObjectFindInfo> ATwinLinkCityObjectTree::FindCityObjectGroups(
    const FTwinLinkSpatialID& SpatialId) const {
    TArray<FCityObjectFindInfo> Ret;
    const auto SpacialWorldBox = SpatialId.GetSpatialIDArea(InstancedCityModel->GeoReference);
    // 領域の範囲外は無視する
    if (RangeWorld.IntersectXY(SpacialWorldBox) == false)
        return Ret;

    auto LowerBoundKeyIndex = [&](const KeyType& Key, int MinIndex = -1, int MaxIndex = -1) {
        return TwinLinkMathEx::LowerBound(
            TreeMap
            , [&](const FTwinLinkCityObjectQuadTreeKeyMap& K) { return K.Key.AsUInt64() > Key.AsUInt64(); }
            , MinIndex
            , MaxIndex);
    };

    const auto Key = ToKey(SpatialId);
    const auto Index = LowerBoundKeyIndex(Key);

    if (Index >= TreeMap.Num())
        return Ret;

    auto Check = [&](int TargetIndex) {
        const auto Start = TargetIndex;
        const auto End = TargetIndex + TreeMap[TargetIndex].SpaceSize;
        for (auto I = Start; I < End; ++I) {
            for (auto& N : TreeMap[I].Nodes) {
                if (IsTarget(N, TreeMap[I].Key.Zoom, SpacialWorldBox)) {
                    Ret.Add(FCityObjectFindInfo(N));
                }
            }
        }
        return End;
    };

    // 対応するインデックスが見つかったらすべての子を1回で探索できる
    if (Index >= 0 && TreeMap[Index].Key == Key) {
        Check(Index);
    }
    // Keyに対応するノードが存在しない場合.
    // Keyに相当する場所から右側を見ていったKeyの子になる空間全部をチェックする
    else {
        for (auto NextIndex = Index + 1; NextIndex < TreeMap.Num(); ) {
            if (Key.IsParentOf(TreeMap[NextIndex].Key) == false)
                break;
            NextIndex = Check(NextIndex);
        }
    }

    // 親ノードから探していく
    auto SearchMax = Index;
    for (auto Z = Key.Zoom - 1; Z >= EntireSpaceKey.Zoom; --Z) {
        const auto ParentKey = Key.ZoomChanged(Z);
        const auto ParentIndex = LowerBoundKeyIndex(ParentKey, 0, SearchMax);
        // 一番左が自分以下の場合はさらに親のノードも存在しない
        if (ParentIndex < 0)
            break;
        // 親がノードを持っている場合はチェックする
        if (TreeMap[ParentIndex].Key == ParentKey) {
            // 親は自分自身だけを見る(子はすでに見ているから
            for (auto& N : TreeMap[ParentIndex].Nodes) {
                if (IsTarget(N, TreeMap[ParentIndex].Key.Zoom, SpacialWorldBox)) {
                    Ret.Add(FCityObjectFindInfo(N));
                }
            }
        }
        // 見つかっていない場合もあるのでParentIndex-1しなくてよい
        SearchMax = ParentIndex;
    }

    return Ret;
}

void ATwinLinkCityObjectTree::GetZoomRange(int& MinZoom, int& MaxZoom) const {
    MinZoom = EntireSpaceKey.Zoom;
    MaxZoom = MAX_ZOOM_LEVEL;
}

ATwinLinkCityObjectTree::KeyType ATwinLinkCityObjectTree::ToKey(const FTwinLinkSpatialID& SpId) const {
    auto Key = KeyType::Create(SpId).ZoomChanged(MAX_ZOOM_LEVEL);
    Key.X -= OffsetKey.X;
    Key.Y -= OffsetKey.Y;
    return Key.ZoomChanged(SpId.GetZ());
}

FTwinLinkSpatialID ATwinLinkCityObjectTree::ToSpatialId(const KeyType& Key) const {
    auto Ret = Key.ZoomChanged(MAX_ZOOM_LEVEL);
    Ret.X += OffsetKey.X;
    Ret.Y += OffsetKey.Y;
    Ret = Ret.ZoomChanged(Key.Zoom);
    return FTwinLinkSpatialID::Create(Ret.Zoom, 0, Ret.X, Ret.Y, false);
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
#if false
        // #TODO : 場合によってはどこにも属さない建物が出てくるのでかなり低めに設定している
        constexpr float AREA_RATE = 0.1f;
        const auto IntersectArea = TwinLinkMathEx::GetIntersectBoxXY(Bb, SpatialWorldBox).GetArea();
        const auto BbArea = TwinLinkMathEx::GetAreaXY(Bb);
        const auto SpatialArea = TwinLinkMathEx::GetAreaXY(SpatialWorldBox);
        return IntersectArea > BbArea * AREA_RATE || IntersectArea > SpatialArea * AREA_RATE;
#else
        return Bb.IntersectXY(SpatialWorldBox);
#endif
    }

    return false;
}

TArray<ATwinLinkCityObjectTree::FCityObjectFindInfo> ATwinLinkCityObjectTree::DebugGetCityObjectGroups(
    const FTwinLinkSpatialID& SpatialId) const {
    TArray<FCityObjectFindInfo> Ret;
    const auto Box = SpatialId.GetSpatialIDArea(InstancedCityModel->GeoReference);

    if (RangeWorld.IntersectXY(Box) == false)
        return Ret;

    for (auto& Tree : TreeMap) {
        for (const auto Item : Tree.Nodes) {
            if (FBox Bb; FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(Item.Get(), Bb)) {
                if (Item.IsValid() == false)
                    continue;
                if (Item->GetVisibleFlag() == false)
                    continue;
                if (Bb.IntersectXY(Box)) {
                    Ret.Add(FCityObjectFindInfo(Item));
                }
            }
        }

    }
    return Ret;
}
