// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkFloorSwitcher.h"
#include "TwinLinkFloorViewPanel.h"

void UTwinLinkFloorSwitcher::TwinLinkFloorSwitcherSetup(const TObjectPtr<UTwinLinkFloorViewPanel>& Panel) {
    FloorUpButton->Button->OnClicked.Clear();
    FloorUpButton->Button->OnClicked.AddDynamic(this, &UTwinLinkFloorSwitcher::OnClickedUp);
    FloorDownButton->Button->OnClicked.Clear();
    FloorDownButton->Button->OnClicked.AddDynamic(this, &UTwinLinkFloorSwitcher::OnClickedDown);

    FloorPanel = Panel;

    FloorSwitch();
}

void UTwinLinkFloorSwitcher::OnClickedDown() {
    if (FloorPanel == nullptr) {
        return;
    }
    FloorPanel->FloorViewChangeNextKey(-1);
}

void UTwinLinkFloorSwitcher::OnClickedUp() {
    if (FloorPanel == nullptr) {
        return;
    }
    FloorPanel->FloorViewChangeNextKey(+1);
}

void UTwinLinkFloorSwitcher::FloorSwitch() {
    if (FloorPanel == nullptr) {
        return;
    }
    const auto Floor = FloorPanel->GetSelectedFloorKey();
    const auto Floors = FloorPanel->GetVisibleFloorKeys();
    const auto Index = Floors.Find(Floor);

    FloorDownButton->Button->SetIsEnabled(Index != 0);
    FloorUpButton->Button->SetIsEnabled(Index != Floors.Num() - 1);
    FloorText->SetText(FText::FromString(Floor.Equals("Exterior") ? TEXT("外観") : Floor));
}