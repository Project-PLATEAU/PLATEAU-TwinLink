// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkColorAdjustmentElement.h"

void UTwinLinkColorAdjustmentElement::OnSelectEdit() {
    ElementRoot->EditMaterial(MaterialIndex);
}