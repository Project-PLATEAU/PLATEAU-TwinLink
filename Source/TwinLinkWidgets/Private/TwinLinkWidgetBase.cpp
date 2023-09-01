// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWidgetBase.h"

#include "TwinLink.h"

void UTwinLinkWidgetBase::NativeConstruct() {
    Super::NativeConstruct();
    SetupLinkageWithAdminMode();
}

void UTwinLinkWidgetBase::NativeDestruct() {
    FinalizeLinkageWithAdminMode();
    Super::NativeDestruct();
}

bool UTwinLinkWidgetBase::IsActiveAdminMode() {
    auto& Mod = FTwinLinkModule::Get();
    return Mod.IsAdminModeEnabled();
}

void UTwinLinkWidgetBase::SetupLinkageWithAdminMode() {
    auto& Mod = FTwinLinkModule::Get();

    // 管理者モード有効化時のイベント登録
    check(OnActiveAdminModeHnd.IsValid() == false);
    OnActiveAdminModeHnd = Mod.OnActiveAdminMode.AddLambda([this]() {
        OnChangedAdminMode(true);
        });

    // 管理者モード無効化時のイベント登録
    check(OnInactiveAdminModeHnd.IsValid() == false);
    OnInactiveAdminModeHnd = Mod.OnInactiveAdminMode.AddLambda([this]() {
        OnChangedAdminMode(false);
        });

    // ウィジェットを同期する
    OnChangedAdminMode(Mod.IsAdminModeEnabled());
}

void UTwinLinkWidgetBase::FinalizeLinkageWithAdminMode() {
    auto& Mod = FTwinLinkModule::Get();

    // 管理者モード有効化時のイベント登録
    check(OnActiveAdminModeHnd.IsValid());
    Mod.OnActiveAdminMode.Remove(OnActiveAdminModeHnd);
    OnActiveAdminModeHnd.Reset();

    // 管理者モード無効化時のイベント登録
    check(OnInactiveAdminModeHnd.IsValid());
    Mod.OnInactiveAdminMode.Remove(OnInactiveAdminModeHnd);
    OnInactiveAdminModeHnd.Reset();

}
