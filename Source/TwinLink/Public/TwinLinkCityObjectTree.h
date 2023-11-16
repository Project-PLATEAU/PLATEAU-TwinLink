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
        int Z = 0;
    /** 東西方向インデックス **/
    UPROPERTY(EditAnywhere)
        int X = 0;
    /** 南北方向インデックス **/
    UPROPERTY(EditAnywhere)
        int Y = 0;
    /*範囲*/
    UPROPERTY(VisibleAnywhere)
        FBox Range;

    // 空間IDに変換する
    FTwinLinkSpatialID ToSpatialID() const {
        return FTwinLinkSpatialID::Create(Z, 0, X, Y, false);
    }
};

/*
 * @brief : 4分木マップのキー. X/Y/Zoom
 */
USTRUCT(BlueprintType)
struct FTwinLinkCityObjectQuadTreeKey {
    GENERATED_BODY();
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        int Zoom = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        int X = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        int Y = 0;

    // 空間IDからとってくる
    static FTwinLinkCityObjectQuadTreeKey Create(const FTwinLinkSpatialID& SpId);

    FTwinLinkCityObjectQuadTreeKey() {}

    FTwinLinkCityObjectQuadTreeKey(int _Zoom, int _X, int _Y)
        : Zoom(_Zoom)
        , X(_X)
        , Y(_Y) {
    }

    /*
     * @brief : 整列用のキーを取得
     */
    uint64 AsUInt64() const;

    /*
     * @brief : 自身がChildの親かどうか判定する
     */
    bool IsParentOf(const FTwinLinkCityObjectQuadTreeKey& Child) const;
    /*
     * @brief : 位置をなるべくキープした状態でズームだけ変更した空間IDを返す
     *        : 引き -> 寄りの場合は位置をキープできる
     *        : 寄り -> 引きの場合は小数点部分は切り捨て
     */
    FTwinLinkCityObjectQuadTreeKey ZoomChanged(int NextZoom) const;

    //// 空間IDに変換
    //FTwinLinkSpatialID ToSpatialId() const {
    //    return FTwinLinkSpatialID::Create(Zoom, 0, X, Y, false);
    //}

    // 
    bool operator==(const FTwinLinkCityObjectQuadTreeKey& Other) const {
        return X == Other.X && Y == Other.Y && Zoom == Other.Zoom;
    }

    bool operator!=(const FTwinLinkCityObjectQuadTreeKey& Other) const {
        return !(*this == Other);
    }

    bool operator<(const FTwinLinkCityObjectQuadTreeKey& Other) const {
        return AsUInt64() < Other.AsUInt64();
    }

    bool operator>(const FTwinLinkCityObjectQuadTreeKey& Other) const {
        return AsUInt64() > Other.AsUInt64();
    }

    FString StringZXY() const;
};

// TMapのキーとして使うためにGetTypeHashを定義する
FORCEINLINE uint32 GetTypeHash(const FTwinLinkCityObjectQuadTreeKey& A) {
    return A.X ^ A.Y ^ A.Zoom;
}


USTRUCT(BlueprintType)
struct FTwinLinkCityObjectBinaryTree {
    GENERATED_BODY();
public:
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectQuadTreeKey Key;

    UPROPERTY(VisibleAnywhere)
        TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> Nodes;
};

USTRUCT(BlueprintType)
struct FTwinLinkCityObjectQuadTree {
    GENERATED_BODY();
public:
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectBinaryTree BinaryTree;

};

USTRUCT(BlueprintType)
struct FTwinLinkCityObjectQuadTreeKeyMap {
    GENERATED_BODY();
public:
    // 自身の空間ID
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectQuadTreeKey Key;

    // この空間ID自体が持っているノード(子空間は含まない
    UPROPERTY(VisibleAnywhere)
        TArray<TWeakObjectPtr<UPLATEAUCityObjectGroup>> Nodes;

    // 自分自身を含めた子空間の数
    UPROPERTY(VisibleAnywhere)
        int SpaceSize = 0;
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
        FTwinLinkCityObjectQuadTreeKey Key;

        FCityObjectFindInfo() {}
        FCityObjectFindInfo(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject)
            : CityObjectGroup(CityObject) {
        }
        FCityObjectFindInfo(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject, FTwinLinkCityObjectQuadTreeKey K)
            : CityObjectGroup(CityObject)
            , Key(K) {
        }
    };

    static constexpr int MAX_ZOOM_LEVEL = FTwinLinkSpatialID::MAX_ZOOM_LEVEL;
    static constexpr int MIN_ZOOM_LEVEL = FTwinLinkSpatialID::MIN_ZOOM_LEVEL;

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
    TArray<FCityObjectFindInfo> FindCityObjectGroups(const FTwinLinkSpatialID& SpatialId) const;

    /*
     * @brief : この領域の最小ズーム/最大ズームを返す
     */
    UFUNCTION(BlueprintPure)
        void GetZoomRange(int& MinZoom, int& MaxZoom) const;

private:
    using KeyType = FTwinLinkCityObjectQuadTreeKey;

    KeyType ToKey(const FTwinLinkSpatialID& SpId) const;

    FTwinLinkSpatialID ToSpatialId(const KeyType& Key) const;

    /*
     * TreeMapのI番目のノードの配列サイズを計算する.戻り値は配列の最後のインデックスの次の値
     */
    int BuildIndexCache(int I);

    /*
     * @brief : 空間IDに属しているかの判定
     */
    static bool IsTarget(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject, int Zoom, const FBox& SpatialWorldBox);

    // デバッグ用
    /*
     * @brief : デバッグ確認用. 全探索してとってくる
     */
    TArray<FCityObjectFindInfo> DebugGetCityObjectGroups(const FTwinLinkSpatialID& SpatialId) const;

    /*
     * @brief : デバッグ描画
     */
    void DebugDraw(float DeltaSeconds);

    /*
     * @brief : デバッグ用. 全探索して格納されているキーを取得する
     */
    std::optional<KeyType> DebugFindKey(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup) const;
private:
    // 範囲外用
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectBinaryTree OutOfRangedTree;

    // 対象の都市モデル
    UPROPERTY(EditAnywhere)
        TWeakObjectPtr<APLATEAUInstancedCityModel> InstancedCityModel;

    // ワールド空間における範囲
    UPROPERTY(VisibleAnywhere)
        FBox RangeWorld;

    // 空間ID座標系における範囲
    UPROPERTY(VisibleAnywhere)
        FBox RangeVoxelSpace;

    // 領域全体を包括する空間ID
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectQuadTreeKey EntireSpaceKey;

    // 領域の原点
    UPROPERTY(VisibleAnywhere)
        FTwinLinkCityObjectQuadTreeKey OffsetKey;

    // (モートン順序+ズームレベル)でソートされたFTwinLinkCityObjectQuadTreeKeyMap
    UPROPERTY(VisibleAnywhere)
        TArray<FTwinLinkCityObjectQuadTreeKeyMap> TreeMap;
private:
    // 以下デバッグ用
    UPROPERTY(EditAnywhere)
        bool DebugShowSpace;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpaceGroup;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpaceDebug;

    UPROPERTY(EditAnywhere)
        bool DebugShowSpaceDebugName;

    UPROPERTY(EditAnywhere)
        bool DebugCheckAllVoxel = false;

    UPROPERTY(EditAnywhere)
        FTwinLinkDebugSpatialId DebugShowSpaceId;

    UPROPERTY(EditAnywhere)
        FString DebugShowCompName;
};
