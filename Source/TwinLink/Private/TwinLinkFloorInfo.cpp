// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFloorInfo.h"

bool UTwinLinkFloorInfo::IsValid(
    const FString& InName, 
    const FString& InCategory,
    const FVector& InLocation,
    const FString& InImageFileName, 
    const FString& InGuideText, 
    const FString& InOpningHoursText) {
    if (InName.IsEmpty()) return false;
    if (InCategory.IsEmpty()) return false;
    return true;
}

void UTwinLinkFloorInfo::Setup(
    const FString& InName,
    const FString& InCategory,
    const FVector& InLocation,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {
    check(UTwinLinkFloorInfo::IsValid(
        InName,
        InCategory,
        InLocation,
        InImageFileName,
        InGuideText,
        InOpningHoursText
    ));

    Name = InName;
    Category = InCategory;
    Location = InLocation;
    ImageFileName = InImageFileName;
    GuideText = InGuideText;
    OpningHoursText = InOpningHoursText;

    // 値が変更された
    BroadcastEvOnChanged();

}

void UTwinLinkFloorInfo::Setup(TArray<FString> DataStr) {
    check(DataStr.Num() == NumDataElement);

    const auto _Location = FVector(FCString::Atof(*DataStr[2]), FCString::Atof(*DataStr[3]), FCString::Atof(*DataStr[4]));
    check(UTwinLinkFloorInfo::IsValid(
        DataStr[0],
        DataStr[1],
        _Location,
        DataStr[5],
        DataStr[6],
        DataStr[7]
    ));

    Name = DataStr[0];
    Category = DataStr[1];
    Location = _Location;
    ImageFileName = DataStr[5];
    GuideText = DataStr[6];
    OpningHoursText = DataStr[7];


    // 値が変更された
    BroadcastEvOnChanged();

}
