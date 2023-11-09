// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "Engine/DataTable.h"
#include "TwinLinkSystemVersionInfo.h"
#include "TwinLinkSmartPoleDataAsset.h"
#include "TwinLinkSmartPolePairDataAsset.h"
#include "TwinLinkWeatherDataSubSystem.generated.h"

/**
 * スマートポールの時刻と値
 */
USTRUCT()
struct FTwinLinkWeatherTimeValue : public FTableRowBase {
    GENERATED_BODY()
public:
    /** 時刻 **/
    FDateTime Time;
    /** コンポーネント名 **/
    FString Value;
public:
    /**
     * @brief デフォルトコンストラクタ
    */
    FTwinLinkWeatherTimeValue() { ; }
    /**
     * @brief コンストラクタ
    */
    FTwinLinkWeatherTimeValue(const FString& NewTime, const FString& NewValue) {
        const TCHAR* PNewTime = *NewTime;
        Time.ImportTextItem(PNewTime, 0, nullptr, nullptr);
        Value = NewValue;
    }
};

/**
 * スマートポール情報取得サブシステム
 */
UCLASS()
class TWINLINK_API UTwinLinkWeatherDataSubSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()
private:
    /** バージョン情報（ダミー） **/
    const TwinLinkSystemVersionInfo VersionInfo = TwinLinkSystemVersionInfo(0, 0, 0);
public:
    /**
     * @brief スマートポールデータをインポート
    */
    void ImportSmartPole(const FString& Directory);
    /**
     * @brief スマートポールデータをパース
    */
    void Parse(const TArray<FString>& StrData);
    /**
     * @brief スマートポール定義をロード
    */
    void LoadSmartPoleDefine();
    /**
     * @brief スマートポール単位をロード
    */
    void LoadSmartPolePair();
    /**
     * @brief スマートポール座標取得
    */
    FVector GetSmartPoleCoodinate(const FString& Device);
public:
    /** スマートポールデータのコレクション（Device<Unit<JulianDay<DayOfTime<Time, Value>>>>） **/
    TMap<FString, TMap<FString, TMap<double, TMap<FTimespan, TMap<FDateTime, FString>>>>> SmartPoleInputs;
    /** スマートポール定義 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkSmartPoleDataAsset> SmartPoleDefines;
    /** スマートポール単位 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkSmartPolePairDataAsset> SmartPolePair;
    /** インポート済みファイル名 **/
    TArray<FString> FileNames;
    /** 表示日 **/
    double CurrentJulianDay;
    /** 表示時刻 **/
    FTimespan CurrentTimeOfDay;
};