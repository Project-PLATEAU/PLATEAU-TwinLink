// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TwinLinkAssetPlacementPresetData.generated.h"

/**
 * プリセットアセット情報
 */
USTRUCT()
struct FTwinLinkAssetPlacementPresetData : public FTableRowBase {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        /** プリセットID **/
        int PresetID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        /** ウィジェット用テクスチャ **/
        TObjectPtr<UTexture2D> WidgetTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        /** スタティックメッシュ **/
        TObjectPtr<UObject> AssetSils;
public:
    FTwinLinkAssetPlacementPresetData() {
        PresetID = 0;
        WidgetTexture = nullptr;
        AssetSils = nullptr;
    }
};
