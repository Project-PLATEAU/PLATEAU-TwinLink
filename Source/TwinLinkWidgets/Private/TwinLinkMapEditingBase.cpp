// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkMapEditingBase.h"
#include "TwinLinkCommon.h"

void UTwinLinkMapEditingBase::NativeConstruct() {
    UTwinLinkTabContentBase::NativeConstruct();
    Setup();
}

void UTwinLinkMapEditingBase::NativeDestruct() {
    Finalize();
    UTwinLinkTabContentBase::NativeDestruct();
}

void UTwinLinkMapEditingBase::Setup() {

}

void UTwinLinkMapEditingBase::Finalize() {

}
