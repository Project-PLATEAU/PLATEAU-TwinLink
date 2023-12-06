// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAssetPlacementPanel.h"
#include "TwinLinkCommon.h"
#include "TwinLinkAssetPlacementSystem.h"

void UTwinLinkAssetPlacementPanel::SetupOnTwinLink() {
    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());
    ObservableCollection = AssetPlacementSys.Get()->GetAssetPlacementInfoCollection().Get();
}

void UTwinLinkAssetPlacementPanel::TwinLinkAssetPlacementExit() {
    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());
    AssetPlacementSys.Get()->TwinLinkAssetPlacementClearUnsettledActor();

    if (CurrentElement != nullptr) {
        CurrentElement->Deselect();
    }
}
