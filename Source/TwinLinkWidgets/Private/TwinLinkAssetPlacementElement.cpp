// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAssetPlacementElement.h"
#include "TwinLinkCommon.h"
#include "TwinLinkAssetPlacementSystem.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"

void UTwinLinkAssetPlacementElement::TwinLinkAssetPlacementElementSetup(const int AssetPresetID, UObject* Obj) {
    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());

    PresetID = AssetPresetID;

    const auto Button = Cast<UButton>(GetRootWidget());//Cast<UButton>(Obj);

    if (Button) {
        Button->WidgetStyle.Normal.SetResourceObject(AssetPlacementSys.Get()->PresetTextures[PresetID]);
        Button->WidgetStyle.Normal.ImageSize = FVector2D(100, 100);
        Button->WidgetStyle.Hovered.SetResourceObject(AssetPlacementSys.Get()->PresetTextures[PresetID]);
        Button->WidgetStyle.Hovered.ImageSize = FVector2D(100, 100);
        Button->WidgetStyle.Pressed.SetResourceObject(AssetPlacementSys.Get()->PresetTextures[PresetID]);
        Button->WidgetStyle.Pressed.ImageSize = FVector2D(100, 100);
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLinkAssetPlacementElementSetup"));
    }
}

void UTwinLinkAssetPlacementElement::TwinLinkAssetPlacementAddPresetAsset() {
    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());

    const auto AssetPlacementInfo = NewObject<UTwinLinkAssetPlacementInfo>();
    const auto bIsSuc = AssetPlacementInfo->Setup(PresetID, FVector(0, 0, 0), FVector(0, 0, 0));
    //check(bIsSuc);
    //AssetPlacementSys.Get()->GetAssetPlacementInfoCollection()->Add(AssetPlacementInfo);
    //AssetPlacementSys.Get()->ExportAssetPlacementInfo();

    ////アセット追加
    //AssetPlacementSys.Get()->AddAssetPlacementActor(AssetPlacementInfo);

    AssetPlacementSys.Get()->SpawnUnsettledActor(AssetPlacementInfo);

    //選択中の枠を表示
    const auto Button = Cast<UButton>(GetRootWidget());

    const auto Image = Cast<UImage>(Button->GetChildAt(0));

    if (Image) {
        Image->SetVisibility(ESlateVisibility::Visible);
    }
}