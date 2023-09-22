// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UnrealWidgetFwd.h"
#include "Modules/ModuleManager.h"
#include <Components/SlateWrapperTypes.h>

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

    /**
    * @brief 管理対象建築物（LOD4）を設定
    * @param 建築物（LOD4）のアクタ
    */
    void SetFacilityModel(const TObjectPtr<AActor> Model) { FacilityModel = Model; }

    /**
    * @brief 管理対象建築物（LOD4）を取得
    * @return 建築物（LOD4）のアクタ
    */
    TObjectPtr<AActor> GetFacilityModel() const { return FacilityModel; }

    /**
    * @brief PLATEAU都市モデルを設定
    * @param 都市のアクタ
    */
    void SetCityModel(const TObjectPtr<AActor> Model) { CityModel = Model; }

    /**
    * @brief PLATEAU都市モデルを取得
    * @return 都市のアクタ
    */
    TObjectPtr<AActor> GetCityModel() const { return CityModel; }

public:
    /** 管理者モード有効化時に呼ばれる **/
    DelEvOnChangedAdminMode OnActiveAdminMode;

    /** 管理者モード無効化時に呼ばれる **/
    DelEvOnChangedAdminMode OnInactiveAdminMode;

private:
    bool bAdminMode = false;

    /** 管理対象建築物（LOD4） **/
    TObjectPtr<AActor> FacilityModel;

    /** PLATEAU都市モデル **/
    TObjectPtr<AActor> CityModel;
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

    /**
    * @brief 管理対象建築物（LOD4）を設定
    * @param 建築物（LOD4）のアクタ
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        static void TwinLinkSetFacilityModel(AActor* Model);

    /**
    * @brief PLATEAU都市モデルを設定
    * @param 都市のアクタ
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        static void TwinLinkSetCityModel(AActor* Model);


    /**
    * @brief true -> Visible, false -> Hiddenを返す.
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        static ESlateVisibility AsSlateVisibility(bool visibility);
};