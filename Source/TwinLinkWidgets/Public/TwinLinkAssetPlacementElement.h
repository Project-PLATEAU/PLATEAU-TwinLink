// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkAssetPlacementElement.generated.h"

class UTwinLinkAssetPlacementPanel;

/**
 *
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkAssetPlacementElement : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**
    * @brief プリセットアセットを登録
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementElementSetup(const int AssetPresetID, UObject* Obj, UTwinLinkAssetPlacementPanel* ParentPanel);

    /**
    * @brief プリセットアセットを登録
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementAddPresetAsset();

    /**
    * @brief 選択解除
    */
    void Deselect();

public:
    /** プリセットID **/
    UPROPERTY()
        int PresetID;

private:
    /** アセット配置パネルへの参照 **/
    TObjectPtr<UTwinLinkAssetPlacementPanel> Parent;
};
