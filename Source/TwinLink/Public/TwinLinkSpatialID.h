// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 空間IDを示す構造体
 * 空間ID関係の補助関数も実装
 */
struct TWINLINK_API FTwinLinkSpatialID
{
public:
	~FTwinLinkSpatialID();

private:
    FTwinLinkSpatialID();

public:
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

private:
    /** 高度が正当な値か **/
    bool bIsValidAltitude;
    /** ズームレベル **/
    int Z;
    /** 鉛直方向インデックス **/
    int F;
    /** 東西方向インデックス **/
    int X;
    /** 南北方向インデックス **/
    int Y;

};
