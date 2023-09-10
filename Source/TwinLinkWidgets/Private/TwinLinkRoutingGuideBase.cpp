// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkRoutingGuideBase.h"
#include "TwinLinkCommon.h"

void UTwinLinkRoutingGuideBase::NativeConstruct() {
    UTwinLinkTabContentBase::NativeConstruct();
    Setup();
}

void UTwinLinkRoutingGuideBase::NativeDestruct() {
    Finalize();
    UTwinLinkTabContentBase::NativeDestruct();
}

void UTwinLinkRoutingGuideBase::Setup() {

}

void UTwinLinkRoutingGuideBase::Finalize() {

}
