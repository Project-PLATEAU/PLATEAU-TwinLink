// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkAnalysisBase.h"
#include "TwinLinkCommon.h"

void UTwinLinkAnalysisBase::NativeConstruct() {
    UTwinLinkTabContentBase::NativeConstruct();
    Setup();
}

void UTwinLinkAnalysisBase::NativeDestruct() {
    Finalize();
    UTwinLinkTabContentBase::NativeDestruct();
}

void UTwinLinkAnalysisBase::Setup() {

}

void UTwinLinkAnalysisBase::Finalize() {

}

