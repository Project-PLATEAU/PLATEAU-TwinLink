// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkSpatialAnalysisUiInfo.generated.h"

USTRUCT(BlueprintType)
struct FTwinLinkSpatialAnalysisUiInfo {
    GENERATED_BODY()
public:
    // 空間ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FTwinLinkSpatialID SpatialId;

    // 空間内に含まれている登録地物情報リスト
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TArray<UTwinLinkFacilityInfo*> FacilityInfos;

    // 総建物数
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        int BuildingCount = 0;

    FTwinLinkSpatialAnalysisUiInfo() {}
};

