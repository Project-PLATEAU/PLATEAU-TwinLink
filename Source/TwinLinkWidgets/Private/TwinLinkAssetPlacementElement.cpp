// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAssetPlacementElement.h"
#include "TwinLinkCommon.h"
#include "TwinLinkAssetPlacementSystem.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"
#include "TwinLinkAssetPlacementPanel.h"

void UTwinLinkAssetPlacementElement::TwinLinkAssetPlacementElementSetup(const int AssetPresetID, UObject* Obj, UTwinLinkAssetPlacementPanel* ParentPanel) {
    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());

    PresetID = AssetPresetID;

    const auto Button = Cast<UButton>(GetRootWidget());

    if (Button) {
        auto Style = Button->GetStyle();

        Style.Normal.SetResourceObject(AssetPlacementSys.Get()->PresetTextures[PresetID]);
        Style.Normal.ImageSize = FVector2D(100, 100);
        Style.Hovered.SetResourceObject(AssetPlacementSys.Get()->PresetTextures[PresetID]);
        Style.Hovered.ImageSize = FVector2D(100, 100);
        Style.Pressed.SetResourceObject(AssetPlacementSys.Get()->PresetTextures[PresetID]);
        Style.Pressed.ImageSize = FVector2D(100, 100);

        Button->SetStyle(Style);
    }

    Parent = ParentPanel;
}

void UTwinLinkAssetPlacementElement::TwinLinkAssetPlacementAddPresetAsset() {
    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());

    AssetPlacementSys.Get()->TwinLinkAssetPlacementClearUnsettledActor();

    if (Parent->CurrentElement != nullptr) {
        Parent->CurrentElement->Deselect();
    }

    const auto AssetPlacementInfo = NewObject<UTwinLinkAssetPlacementInfo>();
    const auto bIsSuc = AssetPlacementInfo->Setup(PresetID, FVector(0, 0, 0), FVector(0, 0, 0), FVector(1.0f, 1.0f, 1.0f));

    AssetPlacementSys.Get()->SpawnUnsettledActor(AssetPlacementInfo);

    //選択中の枠を表示
    const auto Button = Cast<UButton>(GetRootWidget());

    const auto Image = Cast<UImage>(Button->GetChildAt(0));

    if (Image) {
        Image->SetVisibility(ESlateVisibility::Visible);
    }

    Parent->CurrentElement = this;
}

void UTwinLinkAssetPlacementElement::Deselect() {
    //選択中の枠を非表示
    const auto Button = Cast<UButton>(GetRootWidget());

    const auto Image = Cast<UImage>(Button->GetChildAt(0));

    if (Image) {
        Image->SetVisibility(ESlateVisibility::Hidden);
    }
}