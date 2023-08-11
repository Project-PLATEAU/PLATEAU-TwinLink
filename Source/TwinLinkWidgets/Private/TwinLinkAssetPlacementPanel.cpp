// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAssetPlacementPanel.h"
#include "TwinLinkCommon.h"
#include "TwinLinkAssetPlacementSystem.h"

void UTwinLinkAssetPlacementPanel::SetupOnTwinLink() {
    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());
    ObservableCollection = AssetPlacementSys.Get()->GetAssetPlacementInfoCollection().Get();

    //TODO プリセットを読み込む

    //TODO コレクションを読み込み配置する
}

void UTwinLinkAssetPlacementPanel::AddAssetPlacement(const int PlacementPresetID, const FVector& PlacementPosition, const FVector& PlacementRotationEuler) {
    const auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();

    const auto AssetPlacementInfo = NewObject<UTwinLinkAssetPlacementInfo>();
    const auto bIsSuc = AssetPlacementInfo->Setup(PlacementPresetID, PlacementPosition, PlacementRotationEuler);
    check(bIsSuc);
    AssetPlacementSys.Get()->GetAssetPlacementInfoCollection()->Add(AssetPlacementInfo);
    AssetPlacementSys.Get()->ExportAssetPlacementInfo();

    //アセット追加
    AssetPlacementSys.Get()->AddAssetPlacementActor(AssetPlacementInfo);
}