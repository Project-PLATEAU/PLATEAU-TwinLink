// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TwinLinkFloorViewElement.generated.h"

class UTwinLinkFloorViewPanel;

/**
 *　階層表示パネル要素ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkFloorViewElement : public UTwinLinkWidgetBase {
    GENERATED_BODY()

public:
    /**
     * @brief 階層表示パネル要素セットアップ
     * @param Label
     * @param Parent
    */
    void ElementSetup(const FString& Label, TObjectPtr<UTwinLinkFloorViewPanel> Parent);
public:
    /** ボタンウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> ListButton;
    /** 表示テキストウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> FacilityInfoListText;
private:
    /**
     * @brief デリゲート
     * @return
    */
    UFUNCTION()
        void OnSelectElement();

    /** 選択時通知先ウィジェットへの参照 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkFloorViewPanel> ElementRoot;
};
