// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkWeatherDataSubSystem.h"
#include "TwinLink.h"
#include "TwinLinkCommon.h"
#include "TwinLinkCSVImporter.h"
#include "TwinLinkCSVContents.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUGeometry.h"

void UTwinLinkWeatherDataSubSystem::ImportSmartPole(const FString& Directory) {
    //相対パスを絶対パスに変換
    const auto AbsoluteDirectory = FPaths::ConvertRelativePathToFull(Directory);

    TArray<FString> Files;
    FString Ext = TEXT("CSV");
    IFileManager::Get().FindFiles(Files, *AbsoluteDirectory, *Ext);

    for (const auto& File : Files) {
        const auto AbsolutePath = FPaths::Combine(AbsoluteDirectory, File);

        TwinLinkCSVImporter CSVImporter;
        TwinLinkCSVContents::Standard CSVContents(VersionInfo);
        const auto bIsSuc = CSVImporter.ImportCSVOmit(*AbsolutePath, &CSVContents);

        if (bIsSuc == false) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Import CSV : %s"), *AbsolutePath);
            continue;
        }

        const TArray<FString> Keys = { TEXT("time"), TEXT("source"), TEXT("device_name"), TEXT("fragment.series"), TEXT("value"), TEXT("unit") };
        const auto KeyContents = CSVContents.GetKeyContents();
        bool IsEqual = Keys.Num() == KeyContents.Num();
        if (IsEqual) {
            for (const auto& Key : Keys) {
                if (!Key.Equals(KeyContents[&Key - &Keys[0]])) {
                    IsEqual = false;
                    break;
                }
            }
        }
        if (!IsEqual) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Format CSV : %s"), *AbsolutePath);
            continue;
        }

        // データを解析して取得する
        const auto& BodyContents = CSVContents.GetBodyContents();

        for (const auto& BodyContentsElement : BodyContents) {
            if (KeyContents.Num() != BodyContentsElement.Num()) {
                UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Line CSV : %s L%d"), *AbsolutePath, &BodyContentsElement - &BodyContents[0]);
                continue;
            }
            // データを解析する
            Parse(BodyContentsElement);
        }

        FileNames.Add(File);
    }

    LoadSmartPoleDefine();

    LoadSmartPolePair();
}

void UTwinLinkWeatherDataSubSystem::Parse(const TArray<FString>& StrData) {
    const auto& Time = StrData[0];
    const auto& Device = StrData[2];
    const auto& Value = StrData[4];
    const auto& Unit = StrData[5];

    int32 DigitIndex;
    if (!Time.FindLastChar(*TEXT("Z"), DigitIndex)) {
        if (!Time.FindLastChar(*TEXT("+"), DigitIndex)) {
            Time.FindLastChar(*TEXT("-"), DigitIndex);
        }
    }
    const auto UTCTime = Time.Left(DigitIndex) + TEXT("Z");

    FDateTime DateTime;
    FDateTime::ParseIso8601(*UTCTime, DateTime);

    auto JulianDay = FMath::TruncToInt32(DateTime.GetModifiedJulianDay());
    const auto TimeOfDay = DateTime.GetTimeOfDay();
    const auto Hours = FTimespan(TimeOfDay.GetHours(), 0, 0);

    if (SmartPoleInputs.Contains(Device)) {
        if (SmartPoleInputs[Device].Contains(Unit)) {
            if (SmartPoleInputs[Device][Unit].Contains(JulianDay)) {
                if (SmartPoleInputs[Device][Unit][JulianDay][Hours].IsEmpty()) {
                    SmartPoleInputs[Device][Unit][JulianDay][Hours].Add(DateTime, Value);
                }
                else {
                    if (SmartPoleInputs[Device][Unit][JulianDay][Hours].IsEmpty()) {
                        SmartPoleInputs[Device][Unit][JulianDay][Hours].Add(DateTime, Value);
                    }
                    else {
                        bool Found = false;
                        for (const auto& TimeValue : SmartPoleInputs[Device][Unit][JulianDay][Hours]) {
                            if (TimeOfDay < TimeValue.Key.GetTimeOfDay()) {
                                Found = true;
                                break;
                            }
                        }
                        if (Found) {
                            SmartPoleInputs[Device][Unit][JulianDay][Hours].Reset();
                            SmartPoleInputs[Device][Unit][JulianDay][Hours].Add(DateTime, Value);
                        }
                    }
                }
            }
            else {
                TMap<FDateTime, FString> NewTimeValueEmpty;

                TMap<FDateTime, FString> NewTimeValue;
                NewTimeValue.Add(DateTime, Value);

                TMap<FTimespan, TMap<FDateTime, FString>> NewTimeOfDay;
                for (int i = 0; i < 24; i++) {
                    TMap<FDateTime, FString> EmptyTimeValue;
                    NewTimeOfDay.Add(FTimespan(i, 0, 0), NewTimeValueEmpty);
                }
                NewTimeOfDay[Hours] = NewTimeValue;

                SmartPoleInputs[Device][Unit].Add(JulianDay, NewTimeOfDay);
            }
        }
        else {
            TMap<FDateTime, FString> NewTimeValueEmpty;

            TMap<FDateTime, FString> NewTimeValue;
            NewTimeValue.Add(DateTime, Value);

            TMap<FTimespan, TMap<FDateTime, FString>> NewTimeOfDay;
            for (int i = 0; i < 24; i++) {
                TMap<FDateTime, FString> EmptyTimeValue;
                NewTimeOfDay.Add(FTimespan(i, 0, 0), NewTimeValueEmpty);
            }
            NewTimeOfDay[Hours] = NewTimeValue;

            TMap<double, TMap<FTimespan, TMap<FDateTime, FString>>> NewJulianDay;
            NewJulianDay.Add(JulianDay, NewTimeOfDay);

            SmartPoleInputs[Device].Add(Unit, NewJulianDay);
        }
    }
    else {
        TMap<FDateTime, FString> NewTimeValueEmpty;

        TMap<FDateTime, FString> NewTimeValue;
        NewTimeValue.Add(DateTime, Value);

        TMap<FTimespan, TMap<FDateTime, FString>> NewTimeOfDay;
        for (int i = 0; i < 24; i++) {
            TMap<FDateTime, FString> EmptyTimeValue;
            NewTimeOfDay.Add(FTimespan(i, 0, 0), NewTimeValueEmpty);
        }
        NewTimeOfDay[Hours] = NewTimeValue;

        TMap<double, TMap<FTimespan, TMap<FDateTime, FString>>> NewJulianDay;
        NewJulianDay.Add(JulianDay, NewTimeOfDay);

        TMap<FString, TMap<double, TMap<FTimespan, TMap<FDateTime, FString>>>> NewUnit;
        NewUnit.Add(Unit, NewJulianDay);

        SmartPoleInputs.Add(Device, NewUnit);
    }
}

void UTwinLinkWeatherDataSubSystem::LoadSmartPoleDefine() {
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    const auto Path = TEXT("/PLATEAU-TwinLink/DataAssets/DA_SmartPoleLink.DA_SmartPoleLink");
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(Path));
    if (!AssetData.IsValid()) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed LoadSmartPoleDefine : %s"), *Path);
        return;
    }
    SmartPoleDefines = Cast<UTwinLinkSmartPoleDataAsset>(AssetData.GetAsset());
}

void UTwinLinkWeatherDataSubSystem::LoadSmartPolePair() {
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    const auto Path = TEXT("/PLATEAU-TwinLink/DataAssets/DA_SmartPolePair.DA_SmartPolePair");
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(Path));
    if (!AssetData.IsValid()) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed LoadSmartPolePair : %s"), *Path);
        return;
    }
    SmartPolePair = Cast<UTwinLinkSmartPolePairDataAsset>(AssetData.GetAsset());
}

FVector UTwinLinkWeatherDataSubSystem::GetSmartPoleCoodinate(const FString& Device) {
    int Index = -1;
    for (int i = 0; i < SmartPoleDefines->SmartPoleLinkData.Num(); i++) {
        if (SmartPoleDefines->SmartPoleLinkData[i].DeviceName.Equals(Device)) {
            Index = i;
            break;
        }
    }
    if (Index < 0) {
        return FVector();
    }
    const auto CityModel = FTwinLinkModule::Get().GetCityModel();
    FPLATEAUGeoReference GeoReference = Cast<APLATEAUInstancedCityModel>(CityModel)->GeoReference;
    FPLATEAUGeoCoordinate Coordinate;
    Coordinate.Latitude = SmartPoleDefines->SmartPoleLinkData[Index].Latitude;
    Coordinate.Longitude = SmartPoleDefines->SmartPoleLinkData[Index].Longitude;
    Coordinate.Height = SmartPoleDefines->SmartPoleLinkData[Index].Height;
    return UPLATEAUGeoReferenceBlueprintLibrary::Project(GeoReference, Coordinate);
}