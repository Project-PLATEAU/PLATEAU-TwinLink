// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkWeatherDataPanel.h"
#include "TwinLinkCommon.h"
#include "TwinLinkWeatherDataSubSystem.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

void UTwinLinkWeatherDataPanel::TwinLinkWeatherDataGetSource(const FString& Directory) {
    auto WeatherDataSubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkWeatherDataSubSystem>();
    check(WeatherDataSubSystem.IsValid());

    WeatherDataSubSystem.Get()->ImportSmartPole(Directory);

    TwinLinkShowWeatherData();

    SetupTimeSlider();

    for (int i = 1; i < ScrollFileList->GetChildrenCount(); i++) {
        ScrollFileList->RemoveChildAt(1);
    }

    const auto OriginElement = Cast<UTextBlock>(ScrollFileList->GetChildAt(0));

    ScrollFileList->ClearChildren();

    for (int i = 0; i < WeatherDataSubSystem.Get()->FileNames.Num(); i++) {
        const auto Element = DuplicateObject<UTextBlock>(OriginElement, ScrollFileList);
        Element->SetText(FText::FromString(WeatherDataSubSystem.Get()->FileNames[i]));
        ScrollFileList->AddChild(Element);
    }
}

void UTwinLinkWeatherDataPanel::TwinLinkShowWeatherData() {
    auto WeatherDataSubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkWeatherDataSubSystem>();
    check(WeatherDataSubSystem.IsValid());

    FString Path = "/PLATEAU-TwinLink/Blueprints/Widget3D/BP_WeatherDataItem.BP_WeatherDataItem_C";
    TSubclassOf<class AActor> WidgetClass = TSoftClassPtr<AActor>(FSoftObjectPath(*Path)).LoadSynchronous();

    if (WidgetClass == nullptr) {
        return;
    }

    for (const auto& Item : WeatherDataItems) {
        Item->RemoveFromParent();
    }

    WeatherDataItems.Reset();

    for (const auto& SmartPole : SmartPoleActors) {
        SmartPole->Destroy();
    }

    for (const auto& InputDevice : WeatherDataSubSystem.Get()->SmartPoleInputs) {
        FVector Location;
        for (const auto& Def : WeatherDataSubSystem.Get()->SmartPoleDefines->SmartPoleLinkData) {
            if (Def.DeviceName.Equals(InputDevice.Key)) {
                Location = WeatherDataSubSystem.Get()->GetSmartPoleCoodinate(InputDevice.Key);
            }
        }
        const auto SmartPoleActor = GetWorld()->SpawnActor<AActor>(WidgetClass);
        SmartPoleActor->SetActorLocation(Location);
        const auto WidgetComponent = Cast<UWidgetComponent>(SmartPoleActor->GetComponentByClass(UWidgetComponent::StaticClass()));
        auto Item = Cast<UTwinLinkWeatherDataItem>(WidgetComponent->GetUserWidgetObject());

        WeatherDataItems.Add(Item);

        SmartPoleActors.Add(SmartPoleActor);

        Item->Setup(InputDevice.Key);
    }
}

void UTwinLinkWeatherDataPanel::SetupTimeSlider() {
    auto WeatherDataSubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkWeatherDataSubSystem>();
    check(WeatherDataSubSystem.IsValid());

    JulianDays.Reset();

    TArray<double> RedundancyJulian;
    //結合
    for (const auto& InputDevice : WeatherDataSubSystem.Get()->SmartPoleInputs) {
        for (const auto& InputUnit : InputDevice.Value) {
            TArray<double> TempArray;
            InputUnit.Value.GetKeys(TempArray);
            RedundancyJulian.Append(TempArray);
        }
    }
    //ソート
    RedundancyJulian.Sort();
    //抽出
    for (int i = 0; i < RedundancyJulian.Num(); i++) {
        if (JulianDays.IsEmpty() || (JulianDays[JulianDays.Num() - 1] != RedundancyJulian[i])) {
            JulianDays.Add(RedundancyJulian[i]);
        }
    }

    if (TimeOfDays.IsEmpty()) {
        for (int i = 0; i < 24; i++) {
            TimeOfDays.Add(FTimespan(i, 0, 0));
        }
    }

    SliderDay->SetIsEnabled(true);
    SliderDay->OnValueChanged.Clear();
    SliderDay->OnValueChanged.AddDynamic(this, &UTwinLinkWeatherDataPanel::OnValueChangedSliderDay);;

    SliderDay->SetValue(1.0f);
    OnValueChangedSliderDay(1.0f);

    SliderTime->SetIsEnabled(true);
    SliderTime->OnValueChanged.Clear();
    SliderTime->OnValueChanged.AddDynamic(this, &UTwinLinkWeatherDataPanel::OnValueChangedSliderTime);;

    const auto DefaultTimeRate = 9.0f / 23.0f;

    SliderTime->SetValue(DefaultTimeRate);
    OnValueChangedSliderTime(DefaultTimeRate);
}

void UTwinLinkWeatherDataPanel::OnValueChangedSliderDay(float Value) {
    auto WeatherDataSubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkWeatherDataSubSystem>();
    check(WeatherDataSubSystem.IsValid());

    if (JulianDays.IsEmpty()) {
        return;
    }

    int Index = FMath::RoundHalfFromZero(Value * (JulianDays.Num() - 1));

    WeatherDataSubSystem.Get()->CurrentModifiedJulianDay = JulianDays[Index];

    //修正ユリウス日からユリウス日へ変換
    double JulianDay = WeatherDataSubSystem.Get()->CurrentModifiedJulianDay + 2400000.5;

    TextDay->SetText(FText::FromString(FDateTime::FromJulianDay(JulianDay).ToString(TEXT("%Y/%m/%d"))));

    const auto DefaultTimeRate = 9.0f / 23.0f;

    SliderTime->SetValue(DefaultTimeRate);
    OnValueChangedSliderTime(DefaultTimeRate);
}

void UTwinLinkWeatherDataPanel::OnValueChangedSliderTime(float Value) {
    auto WeatherDataSubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkWeatherDataSubSystem>();
    check(WeatherDataSubSystem.IsValid());

    if (TimeOfDays.IsEmpty()) {
        return;
    }

    int Index = FMath::RoundHalfFromZero(Value * (TimeOfDays.Num() - 1));

    WeatherDataSubSystem.Get()->CurrentTimeOfDay = TimeOfDays[Index];

    TextTime->SetText(FText::FromString(WeatherDataSubSystem.Get()->CurrentTimeOfDay.ToString(TEXT("%h:%m:%s")).Replace(TEXT("+"), TEXT(""))));

    for (const auto& Item : WeatherDataItems) {
        Item->Update();
    }
}

void UTwinLinkWeatherDataPanel::TwinLinkWeatherDataSetActive(bool IsShow) {
    for (const auto& SmartPoleActor : SmartPoleActors) {
        SmartPoleActor->SetHidden(!IsShow);
        SmartPoleActor->GetComponentByClass<UStaticMeshComponent>()->SetVisibility(IsShow);
    }
}