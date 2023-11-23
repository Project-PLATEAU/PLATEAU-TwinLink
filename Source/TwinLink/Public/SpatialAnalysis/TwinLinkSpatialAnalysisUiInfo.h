// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkSpatialID.h"
#include "TwinLinkSpatialAnalysisUiInfo.generated.h"

USTRUCT(BlueprintType)
struct FTwinLinkSpatialAnalysisPoiInfo
{
    GENERATED_BODY()
    // POI登録名
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FString PoiName;

    // オブジェクト数
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        int ObjectNum = 0;
};

USTRUCT(BlueprintType)
struct FTwinLinkSpatialAnalysisUiInfo {
    GENERATED_BODY()
public:
    // 空間ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FTwinLinkSpatialID SpatialId;

    // 空間内に含まれている登録地物情報リスト
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TArray<FTwinLinkSpatialAnalysisPoiInfo> PoiInfos;

    // 総建物数
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        int BuildingCount = 0;

    FTwinLinkSpatialAnalysisUiInfo() {}
};

