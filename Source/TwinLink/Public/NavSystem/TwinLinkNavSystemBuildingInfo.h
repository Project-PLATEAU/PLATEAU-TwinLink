// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "TwinLinkFacilityInfo.h"
#include "TwinLinkNavSystemBuildingInfo.generated.h"

USTRUCT(BlueprintType)
struct FTwinLinkNavSystemBuildingInfo {
    GENERATED_BODY()
public:
    // 建物情報
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TWeakObjectPtr<UTwinLinkFacilityInfo> FacilityInfo;
};