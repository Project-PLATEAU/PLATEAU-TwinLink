// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkWeatherDataItem.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "TwinLinkWeatherDataPanel.generated.h"

/**
 * 気象データ表示ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkWeatherDataPanel : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**
     * @brief 気象データ初期化
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkWeatherDataGetSource(const FString& Directory);
    /**
     * @brief 気象データ表示開始
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkShowWeatherData();
    /**
     * @brief スライダ初期化
    */
    void SetupTimeSlider();
    /**
     * @brief デリゲート
    */
    UFUNCTION()
        void OnValueChangedSliderDay(float Value);
    /**
     * @brief デリゲート
    */
    UFUNCTION()
        void OnValueChangedSliderTime(float Value);
    /**
     * @brief 気象データ表示・非表示
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkWeatherDataSetActive(bool IsShow);
public:
    /** 気象情報ウィジェット **/
    UPROPERTY()
        TArray<TObjectPtr<UTwinLinkWeatherDataItem>> WeatherDataItems;
    /** 階層表示パネル要素追加先パネルウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UScrollBox> ScrollFileList;
    /** ウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderDay;
    /** ウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> TextDay;
    /** ウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderTime;
    /** ウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> TextTime;
    /** 日付リスト **/
    UPROPERTY()
        TArray<double> JulianDays;
    /** 時間リスト **/
    UPROPERTY()
        TArray<FTimespan> TimeOfDays;
    /** スマートポールアクタ **/
    UPROPERTY()
        TArray<TObjectPtr<AActor>> SmartPoleActors;
};
