﻿// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityInfo.h"

bool UTwinLinkFacilityInfo::Setup(
    const FString& InName, const FString& InCategory,
    const FString& InFeatureID, const FString& InImageFileName,
    const FString& InDescription, const FString& InSpotInfo) {

    Name = InName;
    Category = InCategory;
    FeatureID = InFeatureID;
    ImageFileName = InImageFileName;
    Description = InDescription;
    SpotInfo = InSpotInfo;

    check(!Name.IsEmpty());
    check(!Category.IsEmpty());
    check(!FeatureID.IsEmpty());

    if (EvOnChanged.IsBound()) {
        EvOnChanged.Broadcast();
    }
    return true;
}

bool UTwinLinkFacilityInfo::Setup(const TArray<FString>& DataStr) {
    const auto NumDataElement = 6;
    ensure(DataStr.Num() == NumDataElement);
    if (DataStr.Num() != NumDataElement) {
        return false;
    }

    Name = DataStr[0];
    Category = DataStr[1];
    FeatureID = DataStr[2];
    ImageFileName = DataStr[3];
    Description = DataStr[4];
    SpotInfo = DataStr[5];

    check(!Name.IsEmpty());
    check(!Category.IsEmpty());
    check(!FeatureID.IsEmpty());

    if (EvOnChanged.IsBound()) {
        EvOnChanged.Broadcast();
    }

    return true;
}
