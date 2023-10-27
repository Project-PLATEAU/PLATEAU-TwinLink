// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/TextBlock.h"
#include "TwinLinkWeatherDataItemElement.generated.h"

/**
 * スマートポールデータ要素ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkWeatherDataItemElement : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /** 項目テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> TextSection;
    /** 値テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> TextValue;
    /** 単位テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> TextUnit;
    /** キー **/
    FString UnitKey;
};
