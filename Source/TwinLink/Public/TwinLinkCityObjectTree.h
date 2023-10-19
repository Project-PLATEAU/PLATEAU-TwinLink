#pragma once
#include "PLATEAUInstancedCityModel.h"
#include "TwinLinkPLATEAUCityModelEx.h"
#include "Math/GenericOctree.h"
#include <optional>

#include "TwinLinkSpatialID.h"
#include "TwinLinkCityObjectTree.generated.h"
class APlayerController;
class USceneComponent;

/*
 * @brief: アウトライナーで編集したいので仮で用意.
 *       　 デバッグ以外では使わない
 */
USTRUCT()
struct FTwinLinkDebugSpatialId {
    GENERATED_BODY()
public:
    /** ズームレベル **/
    UPROPERTY(EditAnywhere)
        int Z;
    /** 東西方向インデックス **/
    UPROPERTY(EditAnywhere)
        int X;
    /** 南北方向インデックス **/
    UPROPERTY(EditAnywhere)
        int Y;
    /*範囲*/
    UPROPERTY(VisibleAnywhere)
        FBox Range;

    // 空間IDに変換する
    FTwinLinkSpatialID ToSpatialID() const {
        return FTwinLinkSpatialID::Create(Z, 0, X, Y, false);
    }
};

USTRUCT(BlueprintType)
struct FTwinLinkCityObjectBinaryTree {
    GENERATED_BODY();
public:
    UPROPERTY(VisibleAnywhere)
        TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> Nodes;
};
USTRUCT(BlueprintType)
struct FTwinLinkCityObjectQuadTreeKeyType {
    GENERATED_BODY();
public:
    int X;
    int Y;
    int Zoom;
    //#TODO : X,Y合わせて58ビットに収まる前提
    int64 AsInt64() const;
};
USTRUCT(BlueprintType)
struct FTwinLinkCityObjectQuadTree {
    GENERATED_BODY();
public:
    using KeyType = FTwinLinkCityObjectQuadTreeKeyType;
public:
    static KeyType ToKey(const FTwinLinkSpatialID& SpId);

    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectBinaryTree BinaryTree;

    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectQuadTreeKeyType Key;
};

/*
 * @brief : APLATEAUInstancedCityModelの中にあるUPLATEAUCityObjectGroupのイテレータ
 */
UCLASS()
class TWINLINK_API ATwinLinkCityObjectTree : public AActor {
    GENERATED_BODY()
public:
    struct Node {
        FVector GetPosition() const;
        FVector GetExtent() const;
        TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
        // ボクセル空間におけるバウンディングボックス
        FBox BoundingBox;

        TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> CityObjects;
    };

    // TOctree2に渡すためのセマンティッククラス
    struct Semantics {
        enum { MaxElementsPerLeaf = 16 };
        enum { MinInclusiveElementsPerNode = 7 };
        enum { MaxNodeDepth = FTwinLinkSpatialID::MAX_ZOOM_LEVEL };

        typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

        /*
         * @brief : Nodeからバウンディングボックスを取得する
         */
        FORCEINLINE static FBoxCenterAndExtent GetBoundingBox(const Node& N) {
            return FBoxCenterAndExtent(N.GetPosition(), N.GetExtent());
        }

        /**
         * @brief: Nodeの==を定義
         */
        FORCEINLINE static bool AreElementsEqual(const Node& A, const Node& B) {
            return A.CityObjectGroup == B.CityObjectGroup;
        }

        FORCEINLINE static void SetElementId(const Node& Element, FOctreeElementId2 Id) {
        }
    };

    void Init(APLATEAUInstancedCityModel* CityModel);

    void Add(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject);

    TWeakObjectPtr<APLATEAUInstancedCityModel> GetInstancedCityModel() const {
        return InstancedCityModel;
    }

    FBox GetRangeWorld() const {
        return RangeWorld;
    }

    FBox GetRangeVoxelSpace() const {
        return RangeVoxelSpace;
    }

public:
    ATwinLinkCityObjectTree();
    virtual void Tick(float DeltaSeconds) override;

    TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> GetCityObjectGroups(const FTwinLinkSpatialID& SpatialId) const;
private:
    void DebugDraw(float DeltaSeconds);

    static int64 ToKey(const FTwinLinkSpatialID& SpId);

    FTwinLinkCityObjectQuadTree* GetOrCreateTree(const FTwinLinkSpatialID& SpId);

    void FindChild(FTwinLinkCityObjectQuadTree::KeyType Key, const FBox& WorldBox, TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>>& Out) const;
    /*
     * @brief : デバッグ確認用. 全探索してとってくる
     */
    TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> DebugGetCityObjectGroups(const FTwinLinkSpatialID& SpatialId) const;

    UPROPERTY(VisibleAnywhere)
        TMap<uint64, FTwinLinkCityObjectQuadTree> QuadTreeMap;

    // 範囲外用
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectBinaryTree OutOfRangedTree;

    UPROPERTY(EditAnywhere)
        TWeakObjectPtr<APLATEAUInstancedCityModel> InstancedCityModel;

    // ワールド空間における範囲
    UPROPERTY(VisibleAnywhere)
        FBox RangeWorld;

    // 空間ID座標系における範囲
    UPROPERTY(VisibleAnywhere)
        FBox RangeVoxelSpace;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpace;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpaceGroup;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpaceDebug;
    UPROPERTY(EditAnywhere)
        FTwinLinkZXY DebugShowSpaceId;

    UPROPERTY(EditAnywhere)
        FString DebugShowCompName;
};
