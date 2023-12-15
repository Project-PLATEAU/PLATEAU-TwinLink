// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkRoutingGuideBase.h"
#include "TwinLinkCommon.h"
#include "TwinLinkWorldViewer.h"

void UTwinLinkRoutingGuideBase::NativeConstruct() {
    UTwinLinkTabContentBase::NativeConstruct();
    Setup();
}

void UTwinLinkRoutingGuideBase::NativeDestruct() {
    Finalize();
    UTwinLinkTabContentBase::NativeDestruct();
}

void UTwinLinkRoutingGuideBase::OnActivated() {
    ATwinLinkWorldViewer::ChangeTwinLinkViewMode(GetWorld(), ETwinLinkViewMode::FreeAutoView);
}

void UTwinLinkRoutingGuideBase::Setup() {

}

void UTwinLinkRoutingGuideBase::Finalize() {

}
