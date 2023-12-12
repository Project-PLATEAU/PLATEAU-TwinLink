// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFloorInfo.h"

#include "TwinLinkCommon.h"

bool UTwinLinkFloorInfo::IsValid(
    const FString& InName,
    const FString& InCategory,
    const FVector& InLocation,
    const FVector2D& InUV,
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
    const FVector2D& InUV,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {
    check(UTwinLinkFloorInfo::IsValid(
        InName,
        InCategory,
        InLocation,
        InUV,
        InImageFileName,
        InGuideText,
        InOpningHoursText
    ));

    Name = InName;
    Category = InCategory;
    Location = InLocation;
    UV = InUV;
    ImageFileName = InImageFileName;
    GuideText = InGuideText;
    OpningHoursText = InOpningHoursText;

    // 値が変更された
    BroadcastEvOnChanged();

}

void UTwinLinkFloorInfo::Setup(TArray<FString> DataStr) {
    if (DataStr.Num() != NumDataElement) {
        UE_TWINLINK_LOG(LogTemp, Error, TEXT("Invalid data element in floow info"));
        return;
    }


    const auto _Location = FVector(FCString::Atof(*DataStr[2]), FCString::Atof(*DataStr[3]), FCString::Atof(*DataStr[4]));
    const auto _UV = FVector2D(FCString::Atof(*DataStr[5]), FCString::Atof(*DataStr[6]));

    if (!UTwinLinkFloorInfo::IsValid(
        DataStr[0],
        DataStr[1],
        _Location,
        _UV,
        DataStr[7],
        DataStr[8],
        DataStr[9]
        )) {
        UE_TWINLINK_LOG(LogTemp, Error, TEXT("Invalid data element in floow info"));
        return;
    }

    Name = DataStr[0];
    Category = DataStr[1];
    Location = _Location;
    UV = _UV;
    ImageFileName = DataStr[7];
    GuideText = DataStr[8];
    OpningHoursText = DataStr[9];


    // 値が変更された
    BroadcastEvOnChanged();

}
