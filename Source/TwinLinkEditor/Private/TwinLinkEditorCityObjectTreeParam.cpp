// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorCityObjectTreeParam.h"

FString UTwinLinkEditorCityObjectTreeParam::GetIdentifierString() const {
    return GetPrimaryAssetId().ToString();
}

FPrimaryAssetId UTwinLinkEditorCityObjectTreeParam::GetPrimaryAssetId() const {
    FPrimaryAssetType ItemType = "TwinLinkEditor";
    return FPrimaryAssetId(ItemType, GetFName());
}

