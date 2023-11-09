// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkColorAdjustmentElement.h"
#include "TwinLinkColorAdjustmentPanel.h"

void UTwinLinkColorAdjustmentElement::OnSelectEdit() {
    ElementRoot->EditMaterial(MaterialIndex);
}