// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWidgetBase.h"

#include "TwinLink.h"

void UTwinLinkWidgetBase::DebugChangeAdminMod() {
    auto& Mod = FTwinLinkModule::Get();

    Mod.SetAdminMode(!Mod.IsAdminModeEnabled());

}

void UTwinLinkWidgetBase::SetupAdminSystem() {
    auto& Mod = FTwinLinkModule::Get();

    // 管理者モード有効化時のイベント登録
    check(OnActiveAdminModeHnd.IsValid() == false);
    OnActiveAdminModeHnd = Mod.OnActiveAdminMode.AddLambda([this]() {
        OnActiveAdministorMode();
        });

    // 管理者モード無効化時のイベント登録
    check(OnInactiveAdminModeHnd.IsValid() == false);
    OnInactiveAdminModeHnd = Mod.OnInactiveAdminMode.AddLambda([this]() {
        OnInactiveAdministratorMode();
        });

    // ウィジェットを同期する
    SyncWidgetToAdminMode(Mod.IsAdminModeEnabled());
}

void UTwinLinkWidgetBase::FinalizeAdminSystem() {
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
