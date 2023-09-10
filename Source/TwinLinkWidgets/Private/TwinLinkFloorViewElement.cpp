// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkFloorViewElement.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TwinLinkFloorViewPanel.h"

void UTwinLinkFloorViewElement::ElementSetup(const FString& Label, TObjectPtr<UTwinLinkFloorViewPanel> Parent) {
    const auto Button = Cast<UButton>(GetRootWidget());

    if (!Button) {
        return;
    }

    const auto Text = Cast<UTextBlock>(Button->GetChildAt(0));

    if (Text) {
        Text->SetText(FText::FromString(Label));
    }

    Button->OnClicked.AddDynamic(this, &UTwinLinkFloorViewElement::OnSelectElement);

    ElementRoot = Parent;
}

void UTwinLinkFloorViewElement::OnSelectElement() {
    ElementRoot->FloorViewChange(this);
}