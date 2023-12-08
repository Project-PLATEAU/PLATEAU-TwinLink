// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/Button.h"
#include "Components/Image.h"
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
    /** ボタン **/
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UButton> AssetPlacementButton;
    /** 枠 **/
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UImage> AssetPlacementButtonFrameImage;
private:
    /** アセット配置パネルへの参照 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkAssetPlacementPanel> Parent;
};
