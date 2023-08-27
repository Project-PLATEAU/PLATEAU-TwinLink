// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorNavSystemParam.h"

FString UTwinLinkEditorNavSystemParam::GetIdentifierString() const {
    return GetPrimaryAssetId().ToString();
}

FPrimaryAssetId UTwinLinkEditorNavSystemParam::GetPrimaryAssetId() const {
    FPrimaryAssetType ItemType = "TwinLinkEditor";
    return FPrimaryAssetId(ItemType, GetFName());
}

