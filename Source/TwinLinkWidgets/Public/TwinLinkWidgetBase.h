// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "TwinLinkWidgetBase.generated.h"

/**
 * 全てのWidgetのベースクラス
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkWidgetBase : public UUserWidget {
    GENERATED_BODY()

public:
    /**
     * @brief テスト用　管理者モードをPinを入力せずに強制的に切り替える
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void DebugChangeAdminMod();


    /**
     * @brief 管理者モード関係の初期設定
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void SetupAdminSystem();

    /**
     * @brief 管理者モード関係の終了時処理
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void FinalizeAdminSystem();

    /**
     * @brief ウィジェットを管理者モードに同期する
     * @param bIsActiveAdminMode 現在の管理者モード
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void SyncWidgetToAdminMode(const bool bIsActiveAdminMode);

    /**
     * @brief 管理者モード有効化時に呼ばれる
     * @param bIsActive 
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnActiveAdministorMode();

    /**
     * @brief 管理者モード無効化時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnInactiveAdministratorMode();

private:
    // 登録デリゲードの管理Handle

    FDelegateHandle OnActiveAdminModeHnd;
    FDelegateHandle OnInactiveAdminModeHnd;

};
