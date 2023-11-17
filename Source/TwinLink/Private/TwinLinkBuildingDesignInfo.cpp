// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkBuildingDesignInfo.h"

bool UTwinLinkBuildingDesignInfo::IsValid(const FString& InImageFileName) {
    return InImageFileName.IsEmpty() == false;
}

void UTwinLinkBuildingDesignInfo::Setup(const FString& InImageFileName) {
    check(IsValid(InImageFileName));
    ImageFileName = InImageFileName;

    // 値が変更された
    BroadcastEvOnChanged();
}

void UTwinLinkBuildingDesignInfo::Setup(TArray<FString> DataStr) {
    check(DataStr.Num() == NumDataElement);

    check(UTwinLinkBuildingDesignInfo::IsValid(
        DataStr[0]
    ));

    ImageFileName = DataStr[0];

    // 値が変更された
    BroadcastEvOnChanged();
}

bool UTwinLinkBuildingDesignInfo::IsValid() const {
    return UTwinLinkBuildingDesignInfo::IsValid(ImageFileName);
}
