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
    /*
     * @brief : 初期化. 指定した都市モデルに含まれる建物を登録する
     */
    void Init(APLATEAUInstancedCityModel* CityModel);

    TWeakObjectPtr<APLATEAUInstancedCityModel> GetInstancedCityModel() const {
        return InstancedCityModel;
    }

    FBox GetRangeWorld() const {
        return RangeWorld;
    }

    FBox GetRangeVoxelSpace() const {
        return RangeVoxelSpace;
    }

    ATwinLinkCityObjectTree();

    virtual void Tick(float DeltaSeconds) override;

    /*
     * 指定した空間IDに含まれる建物情報を取得(ただしVisibleのもののみ)
     */
    TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> GetCityObjectGroups(const FTwinLinkSpatialID& SpatialId) const;

private:
    static int64 ToKey(const FTwinLinkSpatialID& SpId);

    /*
     * @brief : 4分木を検索. 無ければ作成して返す
     */
    FTwinLinkCityObjectQuadTree* GetOrCreateTree(const FTwinLinkSpatialID& SpId);

    /*
     * @brief : 4分木を検索
     */
    const FTwinLinkCityObjectQuadTree* GetTree(const FTwinLinkSpatialID& SpId) const;
    /*
     * @brief : 子空間を探索する
     */
    void FindChild(FTwinLinkCityObjectQuadTree::KeyType Key, const FBox& WorldBox, TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>>& Out) const;

    /*
     * @brief: 建物を空間テーブルに登録
     */
    void Add(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject);

    /*
     * @brief : 空間IDに属しているかの判定
     */
    static bool IsTarget(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject, int Zoom, const FBox& SpatialWorldBox);


    // デバッグ用
    /*
     * @brief : デバッグ確認用. 全探索してとってくる
     */
    TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> DebugGetCityObjectGroups(const FTwinLinkSpatialID& SpatialId) const;

    void DebugDraw(float DeltaSeconds);
private:
    // 空間テーブル
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

private:

    // 以下デバッグ用
    UPROPERTY(EditAnywhere)
        bool DebugShowSpace;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpaceGroup;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpaceDebug;
    UPROPERTY(EditAnywhere)
        FTwinLinkDebugSpatialId DebugShowSpaceId;

    UPROPERTY(EditAnywhere)
        FString DebugShowCompName;
};
