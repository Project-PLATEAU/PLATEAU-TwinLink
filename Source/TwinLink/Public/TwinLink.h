// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "TwinLink.generated.h"

DECLARE_EVENT(FTwinLinkModule, DelEvOnChangedAdminMode)

class TWINLINK_API FTwinLinkModule : public IModuleInterface {
public:
    /** IModuleInterfaceの実装 */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /**
     * @brief シングルトンライクにモジュールにアクセスするためのAPIです。
     * Shutdown中にこの関数を呼び出した場合、モジュールがアンロードされている可能性があるため注意してください。
     *
     * @return ロードされたモジュールのインスタンス
     */
    static FTwinLinkModule& Get() {
        return FModuleManager::LoadModuleChecked<FTwinLinkModule>("TwinLink");
    }

    /**
    * @brief PluginのContentディレクトリのパスを取得します。
    */
    static FString GetContentDir();
    
    /**
    * @brief 管理者モードを切り替えます。
    * @param bEnabled trueであれば管理者モードを有効化、falseであれば無効化します。
    */
    void SetAdminMode(const bool bEnabled);

    /**
    * @brief 管理者モードが有効かどうかを取得します。
    */
    bool IsAdminModeEnabled() const;

public:
    /** 管理者モード有効化時に呼ばれる **/
    DelEvOnChangedAdminMode OnActiveAdminMode;

    /** 管理者モード無効化時に呼ばれる **/
    DelEvOnChangedAdminMode OnInactiveAdminMode;

private:
    bool bAdminMode = false;
};

UCLASS()
class TWINLINK_API UTwinLinkBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    /**
    * @brief 管理者モードを切り替えます。
    * @param bEnabled trueであれば管理者モードを有効化、falseであれば無効化します。
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        static void SetAdminMode(const bool bEnabled);

    /**
    * @brief 管理者モードが有効かどうかを取得します。
    */
    UFUNCTION(BlueprintPure, Category = "TwinLink")
        static bool IsAdminModeEnabled();
};