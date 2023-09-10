// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkViewPointInfo.h"

bool UTwinLinkViewPointInfo::Setup(const FString& ViewPointName, const FVector& ViewPointPosition, const FVector& ViewPointRotationEuler) {
    this->Name = ViewPointName;
    this->Position = ViewPointPosition;
    this->RotationEuler = ViewPointRotationEuler;
    return true;
}

bool UTwinLinkViewPointInfo::Setup(const TArray<FString>& StrInfo) {
    // プリミティブな型単位での要素数 FVectorならfloatが3つなので　3
    const auto NumPrimElementOfViewPointInfoStruct = 7;
    const auto bIsValid = StrInfo.Num() == NumPrimElementOfViewPointInfoStruct;

    // 引数文字列が正しくない
    ensure(bIsValid);
    if (bIsValid == false) {
        this->Name = TEXT("");
        this->Position = FVector(0, 0, 0);
        this->RotationEuler = FVector(0, 0, 0);
        return false;
    }

    // 文字列を構造体メンバーの型に変換
    int RefIdx = 0;
    this->Name = StrInfo[RefIdx]; RefIdx += 1;
    this->Position = FVector(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2])); RefIdx += 3;
    this->RotationEuler = FVector(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2])); RefIdx += 3;

    return true;
}
