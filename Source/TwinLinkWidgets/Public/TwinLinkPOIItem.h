// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/TextBlock.h"
#include "TwinLinkPOIItem.generated.h"

/**
 * POI表示ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkPOIItem : public UTwinLinkWidgetBase
{
    GENERATED_BODY()
public:
    /** 属性テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> AttributeText;
};
