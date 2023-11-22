// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "TwinLinkSpatialID.generated.h"
struct FPLATEAUGeoReference;
struct FPLATEAUGeoCoordinate;
/**
 * 空間IDを示す構造体
 * 空間ID関係の補助関数も実装
 */
USTRUCT(BlueprintType)
struct TWINLINK_API FTwinLinkSpatialID {
    GENERATED_BODY();
public:
    ~FTwinLinkSpatialID();

    FTwinLinkSpatialID();
public:
    // 最小ズームレベル
    static constexpr int MIN_ZOOM_LEVEL = 1;

    // 最大ズームレベル
    static constexpr int MAX_ZOOM_LEVEL = 25;
    /**
     * @brief 正当性のチェック
     * @param InZ ズームレベル
     * @param InF 鉛直方向インデックス
     * @param InX 東西方向インデックス
     * @param InY 南北方向インデックス
     * @param bIsUsingAltitude 鉛直方向インデックスを利用しているか（falseの時 InFは利用されない)
     * @return
    */
    static bool IsValid(int InZ, int InF, int InX, int InY, bool bIsUsingAltitude = false);

    /**
     * @brief 地球表面　緯度経度などの値から空間IDを設定する
     * @param Lat 緯度
     * @param Lng 経度
     * @param Zoom ズームレベル
     * @param Alt 高度
    */
    static FTwinLinkSpatialID Create(double Lat, double Lng, double Zoom, double Alt = 0, bool bIsUsingAltitude = false);

    /*
     * @brief : 緯度/経度/高度/ズームレベルから空間IDの座標系に変換する
     */
    static FVector ToVoxelSpace(const FPLATEAUGeoCoordinate& GeoCoordinate, int Zoom);

    /*
     * @brief : ワールド座標 -> 空間ID座標に変換
     */
    static FVector WorldToVoxelSpace(const FVector& World, FPLATEAUGeoReference& GeoReference, int Zoom = MAX_ZOOM_LEVEL);

    /**
     * @brief 空間IDを設定する
     * @param InZ ズームレベル
     * @param InF 鉛直方向インデックス
     * @param InX 東西方向インデックス
     * @param InY 南北方向インデックス
     * @param bIsUsingAltitude 鉛直方向インデックスを利用しているか（falseの時 InFは利用されない
    */
    static FTwinLinkSpatialID Create(int InZ, int InF, int InX, int InY, bool bIsUsingAltitude = false);

    /**
     * @brief 座標を内包する空間IDを設定する
     * @param GeoReference
     * @param Position
     * @param ZoomLevel
    */
    static FTwinLinkSpatialID Create(class FPLATEAUGeoReference& GeoReference, const FVector& Position, int ZoomLevel, bool bIsUsingAltitude = false);

    /*
     *@brief : ワールドBoxを内包する最小の空間IDを設定する. 最大まで大きくしても無理な場合はfalse
     */
    static bool TryGetBoundingSpatialId(FPLATEAUGeoReference& GeoReference, const FBox& WorldBox, bool bIsUsingAltitude, FTwinLinkSpatialID& Out);

    /*
     * @brief : ワールドBoxを内包する最小の空間IDを設定する. 最大まで大きくしても無理な場合はnullopt
     */
    static std::optional<FTwinLinkSpatialID> GetBoundingSpatialId(FPLATEAUGeoReference& GeoReference, const FBox& WorldBox, bool bIsUsingAltitude);

    /*
     * @brief Z/F/X/Yの形式の文字列からパースする
     */
    static FTwinLinkSpatialID ParseZFXY(const FString& Str);
public:
    /**
     * @brief 高度が有効な値か
     * この状態は鉛直方向インデックスにも適用される　（falseなら鉛直方向インデックスも無効な値）
     * @return
    */
    bool IsValidAltitude() const { return bIsValidAltitude; }

    /**
     * @brief 空間ID　ズームレベル
     * @return
    */
    int GetZ() const { return Z; }
    /**
     * @brief 空間ID　鉛直方向インデックス
     * @return
    */
    int GetF() const { return F; }
    /**
     * @brief 空間ID　東西方向インデックス
     * @return
    */
    int GetX() const { return X; }
    /**
     * @brief 空間ID　南北方向インデックス
     * @return
    */
    int GetY() const { return Y; }

    /**
     * @brief 空間IDの範囲を取得する
     *  高度の情報は含めていないので注意(必ず0)
     *
     * @param GeoReference
     *  memo APLATEAUInstancedCityModelから取得できる
     *
     * @return UE上のワールド座標
    */
    FBox GetSpatialIDArea(FPLATEAUGeoReference& GeoReference) const;

    /*
     * @brief : Z/F/X/Yの文字列に変換
     */
    FString StringZFXY() const;

    /*
     * @brief : 位置をなるべくキープした状態でズームだけ変更した空間IDを返す
     *        : 引き -> 寄りの場合は位置をキープできる
     *        : 寄り -> 引きの場合は小数点部分は切り捨て
     */
    FTwinLinkSpatialID ZoomChanged(int Zoom) const;

    /**
     * @brief ズームアップを行う
     * @param Zoom ズームしたい数値  18->19 なら 1
     * @param InX 細分化された際のボクセルのインデックス 0<=X<=(2のZoom乗-1)　範囲外の値は隣の空間IDが渡される
     * @param InY 
     * @return 
    */
    FTwinLinkSpatialID ZoomUp(int Zoom, int InX, int InY) const;

    /*
     * @brief : 緯度経度高度に変換
     */
    void ToGeoCoordinate(FPLATEAUGeoCoordinate& Out) const;

    /*
     * @brief : ボクセルの北西位置を返す(北半球前提). 自分自身と同じ
     */
    FTwinLinkSpatialID NorthWest() const;

    /*
     * @brief : ボクセルの南東位置を返す(北半球前提). Create(x+1, Y+1)と同じ
     */
    FTwinLinkSpatialID SouthEast() const;

    /*
     * @brief : return FVector(X,Y,F)と同義. Zoomは無視される
     */
    FVector ToVector() const;

    /*
     * @brief: 高さ情報の除いて一致するかチェックする
     */
    bool EqualZXY(const FTwinLinkSpatialID& Other) const
    {
        return X == Other.X
            && Y == Other.Y
            && Z == Other.Z;
    }

    bool operator==(const FTwinLinkSpatialID& Other) const {
        return X == Other.X
            && Y == Other.Y
            && Z == Other.Z
            && F == Other.F
            && bIsValidAltitude == Other.bIsValidAltitude;
    }
    bool operator!=(const FTwinLinkSpatialID& Other) const {
        return !((*this) == Other);
    }
private:
    /** 高度が正当な値か **/
    UPROPERTY(VisibleAnywhere)
        bool bIsValidAltitude;
    /** ズームレベル **/
    UPROPERTY(VisibleAnywhere)
        int Z;
    /** 鉛直方向インデックス **/
    UPROPERTY(VisibleAnywhere)
        int F;
    /** 東西方向インデックス **/
    UPROPERTY(VisibleAnywhere)
        int X;
    /** 南北方向インデックス **/
    UPROPERTY(VisibleAnywhere)
        int Y;
};
