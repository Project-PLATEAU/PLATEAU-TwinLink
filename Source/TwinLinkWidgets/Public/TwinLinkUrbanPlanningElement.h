// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TwinLinkUrbanPlanningElement.generated.h"

/**
 * 凡例表示ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkUrbanPlanningElement : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**  凡例カラー  **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UImage> GuidesColor;
    /** 凡例テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> GuidesText;
};
