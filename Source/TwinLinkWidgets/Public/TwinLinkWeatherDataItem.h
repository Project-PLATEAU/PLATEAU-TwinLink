// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/PanelWidget.h"
#include "TwinLinkWeatherDataItem.generated.h"

/**
 * スマートポールデータ表示ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkWeatherDataItem : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /** ワールド座標 **/
    FVector WorldLocation;
    /** 時刻 **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> TextTime;
    /** 項目リスト **/
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "TwinLink")
        TObjectPtr<UPanelWidget> VerticalBox;
    /** キー **/
    FString DeviceKey;
public:
    /**
     * @brief スマートポールデータ設定
    */
    void Setup(const FString Device);
    /**
     * @brief スマートポールデータ更新
    */
    void Update();
};
