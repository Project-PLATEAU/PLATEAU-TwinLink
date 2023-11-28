// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

// 汎用マクロ

// 標準のログ出力のカテゴリ
// 必要なものはコメントアウトを解除してcppに実装してください

//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogHAL, Log, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogSerialization, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogUnrealMath, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogUnrealMatrix, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogContentComparisonCommandlet, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogNetPackageMap, Warning, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogNetSerialization, Warning, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogMemory, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogProfilingDebugging, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogCore, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogOutputDevice, Log, All);

//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogSHA, Warning, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogStats, Log, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogStreaming, Display, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogInit, Log, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogExit, Log, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogExec, Warning, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogScript, Warning, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogLocalization, Error, All);
//TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogLongPackageNames, Log, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogProcess, Log, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogLoad, Log, All);
TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogVirtualization, Log, All);

TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogTemp, Log, All);

// カスタムしたログ出力のカテゴリ
// 新規に作成した場合はSlackから共有お願いします。
// こちらのカテゴリを使用時はUE_TWINLINK_C_LOG()を使用してください

// 例 TWINLINK_API DECLARE_LOG_CATEGORY_EXTERN(TwinLinkLogCustom, Log, All);


/**
* @brief TwinLink用のログ出力マクロ
* @param CategoryName   UE既存のカテゴリ名　(マクロ内でTwinLinkのカテゴリに変換します)
* @param Verbosity      Fatal/Error/Warning /Display/Log/Verbose/VeryVorbose
* @param Format         フォーマット
* @param ...            値
*/
#define UE_TWINLINK_LOG(CategoryName, Verbosity, Format, ...) \
    UE_LOG(TwinLink##CategoryName, Verbosity, Format, ##__VA_ARGS__)

/**
* @brief TwinLink用のログ出力マクロ　カスタムカテゴリ版
* @param CategoryName   TwinLinkカテゴリ名　(マクロ内でTwinLinkのカテゴリに変換します)
* @param Verbosity      Fatal/Error/Warning /Display/Log/Verbose/VeryVorbose
* @param Format         フォーマット
* @param ...            値
*/
#define UE_TWINLINK_C_LOG(CategoryName, Verbosity, Format, ...) \
    UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__)


/**
 * @brief TwinLinkで使用するサブシステムに向けたヘルパークラス
*/
TWINLINK_API class TwinLinkSubSystemHelper {
public:
    template<class SubSystemType>
    static TWeakObjectPtr<SubSystemType> GetInstance() {
        FWorldContext* context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
        const auto GameInstance = Cast<UGameInstance>(context->OwningGameInstance);
        return GameInstance->GetSubsystem<SubSystemType>();
    }

};

TWINLINK_API class TwinLinkWorldViewerHelper {
public:

    static TWeakObjectPtr<ACharacter> GetInstance(UWorld* World);

};

TWINLINK_API class TwinLinkGraphicsEnv {
public:
    /**
     * @brief エミッシブの強化係数を取得する
     * 2023/11/20時点では 昼夜によって変化する
     * @param visibility
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    static float GetEmissiveBoostFromEnv(UWorld* World);

    /**
     * @brief 昼夜の状態を0 ~ 1の間で返す(1が夜)
     *        #NOTE : 現状は0.0 or 1.0の2択
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        static float GetNightIntensity(UWorld* World);

};
