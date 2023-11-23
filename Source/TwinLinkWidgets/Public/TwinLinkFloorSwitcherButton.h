// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/Button.h"
#include "TwinLinkFloorSwitcherButton.generated.h"

/**
 * 階層切り替えボタンウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkFloorSwitcherButton : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /** ボタン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> Button;
};
