// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkFloorViewElement.h"
#include "TwinLinkFloorViewPanel.h"

void UTwinLinkFloorViewElement::ElementSetup(const FString& Label, TObjectPtr<UTwinLinkFloorViewPanel> Parent) {
    FacilityInfoListText->SetText(FText::FromString(Label));

    ListButton->OnClicked.AddDynamic(this, &UTwinLinkFloorViewElement::OnSelectElement);
    
    ElementRoot = Parent;
}

void UTwinLinkFloorViewElement::OnSelectElement() {
    ElementRoot->FloorViewChange(this);
}