// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkFloorViewElement.h"

#include "TwinLinkCommon.h"
#include "TwinLinkFloorInfoSystem.h"
#include "TwinLinkFloorViewPanel.h"

void UTwinLinkFloorViewElement::ElementSetup(const FString& Label, TObjectPtr<UTwinLinkFloorViewPanel> Parent, bool CanChangeFloorVisible) {
    FacilityInfoListText->SetText(FText::FromString(Label));

    ListButton->OnClicked.AddDynamic(this, &UTwinLinkFloorViewElement::OnSelectElement);
    ButtonVisible->OnClicked.AddDynamic(this, &UTwinLinkFloorViewElement::OnClickButtonVisible);
    ElementRoot = Parent;
    bCanChangeFloorVisible = CanChangeFloorVisible;
}

void UTwinLinkFloorViewElement::SetFloorVisible(bool Visible) {
    if (CanChangeFloorVisible() == false)
        return;
    bFloorVisible = Visible;
    OnSetFloorVisible(IsFloorVisible());
}

bool UTwinLinkFloorViewElement::IsFloorVisible() const {
    // 表示設定変更できないフロアは常にtrue扱いにする
    return bFloorVisible || bCanChangeFloorVisible == false;
}

bool UTwinLinkFloorViewElement::CanChangeFloorVisible() const {
    return bCanChangeFloorVisible;
}

void UTwinLinkFloorViewElement::OnSelectElement() {
    ElementRoot->FloorViewChange(this);
}

void UTwinLinkFloorViewElement::OnClickButtonVisible() {
    SetFloorVisible(!IsFloorVisible());
    if (ElementRoot)
        ElementRoot->OnChangeFloorVisible(this, IsFloorVisible());
}
