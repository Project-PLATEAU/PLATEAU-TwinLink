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

/*
 * @brief : 4分木マップのキー. X/Y/Zoom
 */
USTRUCT(BlueprintType)
struct FTwinLinkCityObjectQuadTreeKey {
    GENERATED_BODY();
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        int X;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        int Y;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        int Zoom;

    // 空間IDからとってくる
    static FTwinLinkCityObjectQuadTreeKey Create(const FTwinLinkSpatialID& SpId);

    // 
    bool operator==(const FTwinLinkCityObjectQuadTreeKey& Other) const {
        return X == Other.X && Y == Other.Y && Zoom == Other.Zoom;
    }

    bool operator!=(const FTwinLinkCityObjectQuadTreeKey& Other) const {
        return !(*this == Other);
    }

    bool operator<(const FTwinLinkCityObjectQuadTreeKey& Other) const {
        // 同じズームのものは近くに配置してほしいのでまずズームで比較する
        if (Zoom < Other.Zoom)
            return true;
        if (X < Other.X)
            return true;
        if (Y < Other.Y)
            return true;
        return false;
    }
};

// TMapのキーとして使うためにGetTypeHashを定義する
FORCEINLINE uint32 GetTypeHash(const FTwinLinkCityObjectQuadTreeKey& A) {
    return A.X ^ A.Y ^ A.Zoom;
}

USTRUCT(BlueprintType)
struct FTwinLinkCityObjectQuadTree {
    GENERATED_BODY();
public:
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectBinaryTree BinaryTree;
};

/*
 * @brief : APLATEAUInstancedCityModelの中にあるUPLATEAUCityObjectGroupを空間IDで保持する
 */
UCLASS()
class TWINLINK_API ATwinLinkCityObjectTree : public AActor {
    GENERATED_BODY()
public:
    struct FCityObjectFindInfo {
        // 
        TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
        FTwinLinkSpatialID SpatialId;

        FCityObjectFindInfo() {}
        FCityObjectFindInfo(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject, FTwinLinkSpatialID SpId)
            : CityObjectGroup(CityObject)
            , SpatialId(SpId) {
        }
    };

    static ATwinLinkCityObjectTree* Instance(const UWorld* World);

    /*
     * @brief : 初期化. 指定した都市モデルに含まれる建物を登録する
     */
    void Init(APLATEAUInstancedCityModel* CityModel);

    TWeakObjectPtr<APLATEAUInstancedCityModel> GetInstancedCityModel() const {
        return InstancedCityModel;
    }

    FPLATEAUGeoReference* GetGeoReference() const;

    // 都市モデル全体のバウンディングボックス
    FBox GetRangeWorld() const {
        return RangeWorld;
    }

    // GetRangeWorldをMAX_ZOOM_LEVELの空間IDに変換したもの
    FBox GetRangeVoxelSpace() const {
        return RangeVoxelSpace;
    }

    ATwinLinkCityObjectTree();

    virtual void Tick(float DeltaSeconds) override;

    /*
     * 指定した空間IDに含まれる建物情報を取得(ただしVisibleのもののみ)
     */
    TArray<FCityObjectFindInfo> GetCityObjectGroups(const FTwinLinkSpatialID& SpatialId) const;

    /*
     * 指定したワールド座標とズームレベルで計算される空間IDに含まれる建物情報を取得(ただしVisibleのもののみ)
     */
    TArray<FCityObjectFindInfo> GetCityObjectGroups(const FVector& WorldPosition, int Zoom) const;
private:
    using KeyType = FTwinLinkCityObjectQuadTreeKey;

    static auto ToKey(const FTwinLinkSpatialID& SpId) -> KeyType;

    /*
     * @brief : 4分木を検索. 無ければ作成して返す
     */
    FTwinLinkCityObjectQuadTree* GetOrCreateTree(const FTwinLinkSpatialID& SpId);

    /*
     * @brief : 4分木を検索
     */
    const FTwinLinkCityObjectQuadTree* GetTree(const FTwinLinkSpatialID& SpId) const;

    /*
     * @brief : 4分木を検索
     */
    const FTwinLinkCityObjectQuadTree* GetTree(const FTwinLinkCityObjectQuadTreeKey& Key) const;

    /*
     * @brief : 子空間を探索する
     */
    void FindChild(const FTwinLinkSpatialID& SpId, const FBox& WorldBox, TArray<FCityObjectFindInfo>& Out) const;

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

    /*
     * @brief : デバッグ描画
     */
    void DebugDraw(float DeltaSeconds);
private:
    // 空間テーブル
    UPROPERTY(VisibleAnywhere)
        TMap<FTwinLinkCityObjectQuadTreeKey, FTwinLinkCityObjectQuadTree> QuadTreeMap;

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
