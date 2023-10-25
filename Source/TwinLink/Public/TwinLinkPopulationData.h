// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once
#include <TwinLinkSpatialID.h>
#include "TwinLinkPopulationValue.h"

#include "TwinLinkPopulationData.generated.h"

USTRUCT(BlueprintType)
struct FTwinLinkPopulationData {
    GENERATED_BODY();
public:
    // 対応する空間ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FTwinLinkSpatialID SpatialId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FString Type;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TArray<FTwinLinkPopulationValue> Values;
};