// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkAssetPlacementElement.generated.h"

/**
 * 
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkAssetPlacementElement : public UTwinLinkWidgetBase
{
    GENERATED_BODY()

    /**
    * @brief プリセットアセットを登録
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementElementSetup(const int AssetPresetID, UObject* Obj);

    /**
    * @brief プリセットアセットを登録
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementAddPresetAsset();

public:
    /** プリセットID **/
    UPROPERTY()
        int PresetID;
};
