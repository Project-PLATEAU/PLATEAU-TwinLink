// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkUrbanPlanningPanel.generated.h"

/**
 *　都市計画決定情報表示ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkUrbanPlanningPanel : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /** 容積率表示ウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> RateText;
    /** 用途表示ウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> UsageText;
    /** 凡例親ウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UScrollBox> GuidesParent;
    /**
     * @brief 凡例親ウィジェットを更新
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkUrbanPlanningUpdateGuide();
};
