// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAssetPlacementInfo.h"

bool UTwinLinkAssetPlacementInfo::Setup(const int PlacementPresetID, const FVector& PlacementPosition, const FVector& PlacementRotationEuler) {
    this->PresetID = PlacementPresetID;
    this->Position = PlacementPosition;
    this->RotationEuler = PlacementRotationEuler;
    return true;
}

bool UTwinLinkAssetPlacementInfo::Setup(const TArray<FString>& StrInfo) {
    // プリミティブな型単位での要素数 FVectorならfloatが3つなので　3
    constexpr auto NumPrimElementOfViewPointInfoStruct = 7;
    const auto bIsValid = StrInfo.Num() == NumPrimElementOfViewPointInfoStruct;

    // 引数文字列が正しくない
    ensure(bIsValid);
    if (bIsValid == false) {
        this->PresetID = 0;
        this->Position = FVector(0, 0, 0);
        this->RotationEuler = FVector(0, 0, 0);
        return false;
    }

    // 文字列を構造体メンバーの型に変換
    int RefIdx = 0;
    this->PresetID = FCString::Atoi(*StrInfo[RefIdx]); RefIdx += 1;
    this->Position = FVector(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2])); RefIdx += 3;
    this->RotationEuler = FVector(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2])); RefIdx += 3;

    return true;
}