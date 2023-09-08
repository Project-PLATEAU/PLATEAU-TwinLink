// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkBuildingGuideBase.h"
#include "TwinLinkCommon.h"

void UTwinLinkBuildingGuideBase::NativeConstruct() {
    UTwinLinkTabContentBase::NativeConstruct();
    Setup();
}

void UTwinLinkBuildingGuideBase::NativeDestruct() {
    Finalize();
    UTwinLinkTabContentBase::NativeDestruct();
}

void UTwinLinkBuildingGuideBase::Setup() {

}

void UTwinLinkBuildingGuideBase::Finalize() {

}
