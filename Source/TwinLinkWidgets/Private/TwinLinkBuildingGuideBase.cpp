// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkBuildingGuideBase.h"
#include "TwinLinkCommon.h"

#include "TwinLinkFloorInfoSystem.h"

void UTwinLinkBuildingGuideBase::NativeConstruct() {
    UTwinLinkTabContentBase::NativeConstruct();
    Setup();
}

void UTwinLinkBuildingGuideBase::NativeDestruct() {
    Finalize();
    UTwinLinkTabContentBase::NativeDestruct();
}

bool UTwinLinkBuildingGuideBase::CheckIsSelectedGrpID(const FString& InGrpID) {
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    return InfoSys->GetGrpIDBySelectedFloor().Equals(InGrpID, ESearchCase::Type::CaseSensitive);
}

void UTwinLinkBuildingGuideBase::Setup() {

}

void UTwinLinkBuildingGuideBase::Finalize() {

}
