// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityInfoViewBase.h"
#include "TwinLinkCommon.h"

#include "TwinLinkPersistentPaths.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkFacilityInfo.h"

void UTwinLinkFacilityInfoViewBase::Setup(UTwinLinkFacilityInfo* Info) {
    if (FacilityInfo.IsValid()) {
        check(EvOnChangedHnd.IsValid());
        FacilityInfo->EvOnChanged.Remove(EvOnChangedHnd);
    }

    FacilityInfo = Info;
    EvOnChangedHnd = FacilityInfo->EvOnChanged.AddLambda([this]() {
        OnChangedInfo();
        });

    OnChangedInfo();
}

FString UTwinLinkFacilityInfoViewBase::CreateImageFilePath(const FString& InFileName) {
    return TwinLinkPersistentPaths::CreateFacilityImagePath(InFileName);
}
