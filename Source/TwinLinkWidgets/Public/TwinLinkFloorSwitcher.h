// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/TextBlock.h"
#include "TwinLinkFloorSwitcherButton.h"
#include "TwinLinkFloorSwitcher.generated.h"

class UTwinLinkFloorViewPanel;

/**
 *　階層切り替えウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkFloorSwitcher : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /** 階層テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> FloorText;
    /** 上ボタン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTwinLinkFloorSwitcherButton> FloorUpButton;
    /** 下ボタン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTwinLinkFloorSwitcherButton> FloorDownButton;
public:
    /**
    * @brief 初期化
     * @param Panel
    */
    void TwinLinkFloorSwitcherSetup(const TObjectPtr<UTwinLinkFloorViewPanel>& Panel);
    /**
    * @brief 下ボタン選択時デリゲート
    */
    UFUNCTION()
        void OnClickedDown();
    /**
    * @brief 上ボタン選択時デリゲート
    */
    UFUNCTION()
        void OnClickedUp();
    /**
    * @brief 階層切り替え処理
    */
    void FloorSwitch();
private:
    /** 階層選択パネルへの参照 **/
    TObjectPtr<UTwinLinkFloorViewPanel> FloorPanel;
};
