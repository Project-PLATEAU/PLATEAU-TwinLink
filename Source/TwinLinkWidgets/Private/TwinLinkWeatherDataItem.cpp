// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkWeatherDataItem.h"
#include "TwinLinkCommon.h"
#include "TwinLinkWeatherDataSubSystem.h"
#include "Components/HorizontalBox.h"
#include "Kismet/GameplayStatics.h"
#include "TwinLinkWeatherDataItemElement.h"

void UTwinLinkWeatherDataItem::Setup(const FString Device) {
    auto WeatherDataSubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkWeatherDataSubSystem>();
    check(WeatherDataSubSystem.IsValid());

    FString Path = "/PLATEAU-TwinLink/Widgets/Analysis/WBP_WeatherDataItemElement.WBP_WeatherDataItemElement_C";
    TSubclassOf<class UUserWidget> WidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(*Path)).LoadSynchronous();

    if (WidgetClass == nullptr) {
        return;
    }

    DeviceKey = Device;

    VerticalBox->ClearChildren();

    for (const auto& UnitPair : WeatherDataSubSystem.Get()->SmartPolePair->SmartPoleUnitPair) {
        if (WeatherDataSubSystem.Get()->SmartPoleInputs[Device].Contains(UnitPair.Unit)) {
            const auto Element = Cast<UTwinLinkWeatherDataItemElement>(CreateWidget<UUserWidget>(GetWorld(), WidgetClass));
            Element->TextSection->SetText(FText::FromString(UnitPair.DescriptionSection));
            Element->TextUnit->SetText(FText::FromString(UnitPair.DescriptionUnit));
            Element->UnitKey = UnitPair.Unit;
            VerticalBox->AddChild(Element);
        }
    }

    bool IsFound = false;
    for (const auto& DeviceDefine : WeatherDataSubSystem.Get()->SmartPoleDefines->SmartPoleLinkData) {
        if (DeviceDefine.DeviceName.Equals(Device)) {
            IsFound = true;
            break;
        }
    }

    SetVisibility(IsFound ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
}

void UTwinLinkWeatherDataItem::Update() {
    auto WeatherDataSubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkWeatherDataSubSystem>();
    check(WeatherDataSubSystem.IsValid());

    bool IsFound = false;

    for (int i = 0; i < VerticalBox->GetChildrenCount(); i++) {
        const auto Element = Cast<UTwinLinkWeatherDataItemElement>(VerticalBox->GetChildAt(i));
        if (WeatherDataSubSystem.Get()->SmartPoleInputs[DeviceKey][Element->UnitKey].Contains(WeatherDataSubSystem.Get()->CurrentJulianDay)) {
            if (WeatherDataSubSystem.Get()->SmartPoleInputs[DeviceKey][Element->UnitKey][WeatherDataSubSystem.Get()->CurrentJulianDay].Contains(WeatherDataSubSystem.Get()->CurrentTimeOfDay)) {
                FString Value;
                FDateTime Time;
                for (const auto& TimeValue : WeatherDataSubSystem.Get()->SmartPoleInputs[DeviceKey][Element->UnitKey][WeatherDataSubSystem.Get()->CurrentJulianDay][WeatherDataSubSystem.Get()->CurrentTimeOfDay]) {
                    Value = TimeValue.Value;
                    Time = TimeValue.Key;
                }
                Element->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
                Element->TextValue->SetText(FText::FromString(Value));
                IsFound = true;

                TextTime->SetText(FText::FromString(Time.ToString(TEXT("%Y/%m/%d %H:%M:%S:%s"))));
            }
            else {
                Element->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
        else {
            Element->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    SetVisibility(IsFound ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
}