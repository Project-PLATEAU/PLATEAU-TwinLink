// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"
#include "plateau/geometry/geo_coordinate.h"
#include "TwinLinkNavSystemFindPathInfo.generated.h"

/*
 * @brief : パス検索結果の情報
 */
USTRUCT(BlueprintType)
struct FTwinLinkNavSystemFindPathInfo {
    GENERATED_BODY()
public:
    // ナビメッシュ側の検索結果
    FPathFindingResult PathFindResult;

    // 高さチェックを行った後のパスポイント
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TArray<FVector> HeightCheckedPoints = {};

    // HeightCheckedPointsのうち曲がり角のインデックス(元のPathFindResult->GetPathPointsの高さチェックをした座標と同じになる)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TArray<int> PathPointsIndices = {};

    // 総移動距離
    float GetTotalLength() const;

    // 現在高さチェックを行っているインデックス
    int32_t NowHeightCheckIndex = 0;

    FTwinLinkNavSystemFindPathInfo(FPathFindingResult&& Result);
    FTwinLinkNavSystemFindPathInfo() = default;

    // リクエスト検索中
    bool IsRequesting() const;
    // 有効なパスかどうか
    bool IsValid() const;
    // すべてが終わって準備完了かどうか
    bool IsSuccess() const;
    /*
     *@brief : 更新処理. 道路メッシュに対してレイを飛ばして高さを計算していく(処理落ち対策のためにUpdateで回している)
     */
    void Update(const UWorld* World, ECollisionChannel Channel, float MinZ, float MaxZ, int32_t CheckNum);
};

USTRUCT(BlueprintType)
struct FTwinLinkNavSystemOutputPathInfo {
    GENERATED_BODY()
public:
    // 開始地点
    plateau::geometry::GeoCoordinate Start;
    // 終了地点
    plateau::geometry::GeoCoordinate End;
    // 途中経路
    TArray<plateau::geometry::GeoCoordinate> Route;
    // 来るまでかかる時間
    int MinutesByCar;
    // 歩きでかかる時間
    int MinutesByWalk;
};