// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkTabContentBase.h"

void UTwinLinkTabContentBase::NativeConstruct() {
    UTwinLinkWidgetBase::NativeConstruct();
    bIsActive = false;
}

void UTwinLinkTabContentBase::NativeDestruct() {
    UTwinLinkWidgetBase::NativeDestruct();
}

void UTwinLinkTabContentBase::SetActive(bool bInIsActive) {
    // 同じなので処理を行わない
    if (bIsActive == bInIsActive)
        return;

    bIsActive = bInIsActive;

    // CPPコールバック
    if (bIsActive)
        OnActivated();
    else
        OnDeactivate();

    // BPイベント
    OnChangedActivity(bIsActive);
}
