// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TwinLinkAssetPlacementPresetData.h"
#include "TwinLinkAssetPlacementDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class TWINLINK_API UTwinLinkAssetPlacementDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    /**  **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinLink")
        TArray<FTwinLinkAssetPlacementPresetData> PresetProps;
    /**  **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinLink")
        TArray<FTwinLinkAssetPlacementPresetData> PresetInteriors;
};
