// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityInfo.h"

bool UTwinLinkFacilityInfo::IsValid(const FString& InName, const FString& InCategory, const FString& InFeatureID, const FString& InImageFileName, const FString& InDescription, const FString& InSpotInfo, const TArray<FVector>& Entrance) {
    if (InName.IsEmpty()) return false;
    if (InCategory.IsEmpty()) return false;
    if (InFeatureID.IsEmpty()) return false;
    return true;
}

bool UTwinLinkFacilityInfo::Setup(
    const FString& InName, const FString& InCategory,
    const FString& InFeatureID, const FString& InImageFileName,
    const FString& InDescription, const FString& InSpotInfo, const TArray<FVector>& Entrance) {

    check(IsValid(
        InName, 
        InCategory, 
        InFeatureID, 
        InImageFileName, 
        InDescription, 
        InSpotInfo, 
        Entrance));

    Name = InName;
    Category = InCategory;
    FeatureID = InFeatureID;
    ImageFileName = InImageFileName;
    Description = InDescription;
    SpotInfo = InSpotInfo;
    Entrances = Entrance;

    BroadcastEvOnChanged();

    return true;
}

bool UTwinLinkFacilityInfo::Setup(const TArray<FString>& DataStr) {
    const auto NumDataElement = 6;
    // 後方互換のために不等号で比較する
    ensure(DataStr.Num() >= NumDataElement);
    if (DataStr.Num() < NumDataElement) {
        return false;
    }

    Name = DataStr[0];
    Category = DataStr[1];
    FeatureID = DataStr[2];
    ImageFileName = DataStr[3];
    Description = DataStr[4];
    SpotInfo = DataStr[5];

    FString EStr = TEXT("");
    if(DataStr.Num() >= 7)
        EStr = DataStr[6];

    Entrances.Reset();
    if(EStr.IsEmpty() == false)
    {
        TArray<FString> Elems;
        EStr.ParseIntoArray(Elems, TEXT("#"));
        if(Elems.Num() == 3)
        {
            // すべて数値かチェック
            if(Elems.ContainsByPredicate([](const FString& S) { return S.IsNumeric() == false; }) == false)
            {
                auto X = FCString::Atod(*Elems[0]);
                auto Y = FCString::Atod(*Elems[1]);
                auto Z = FCString::Atod(*Elems[2]);
                Entrances.Add(FVector(X, Y, Z));
            }            
        }
    }

    check(!Name.IsEmpty());
    check(!Category.IsEmpty());
    check(!FeatureID.IsEmpty());

    BroadcastEvOnChanged();

    return true;
}

void UTwinLinkFacilityInfo::SetEntrances(const TArray<FVector>& Value) {
    Entrances = Value;
}

bool UTwinLinkFacilityInfo::TryGetFirstEntrance(FVector& Out) const {
    if (Entrances.IsEmpty())
        return false;
    Out = Entrances[0];
    return true;
}
