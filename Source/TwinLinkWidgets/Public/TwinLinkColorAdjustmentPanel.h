// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/ScrollBox.h"
#include "TwinLinkColorAdjustmentEdit.h"
#include "PLATEAUInstancedCityModel.h"
#include "TwinLinkColorAdjustmentPanel.generated.h"

/**
 * マテリアル選択ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkColorAdjustmentPanel : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**
     * @brief マテリアル調整画面初期化
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkColorAdjustmentInit();
    /**
     * @brief マテリアル編集通知
    */
    UFUNCTION()
        void EditMaterial(int MaterialIndex);
    /**
     * @brief　階層選択時通知
    */
    UFUNCTION()
        void OnSelectFloor(UPLATEAUCityObjectGroup* Value);
    /**
     * @brief マテリアル調整画面終了
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkColorAdjustmentExit();
private:
    /**  一覧ウィジェット  **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UScrollBox> Summary;
    /**
     * @brief 階層選択時マテリアル絞り込み
    */
    void SummaryFilter(const TArray<int>& EnableList);
public:
    /**  編集ウィジェット  **/
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UTwinLinkColorAdjustmentEdit> Editor;
};
