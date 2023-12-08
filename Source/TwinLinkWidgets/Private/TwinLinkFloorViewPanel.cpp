// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkFloorViewPanel.h"
#include "TwinLinkCommon.h"

#include "TwinLinkFloorViewElement.h"
#include "Components/PanelWidget.h"
#include "TwinLink.h"

#include "TwinLinkFloorInfoSystem.h"
#include "TwinLinkFloorSwitcher.h"
#include "Components/ScrollBox.h"

#include "TwinLinkAssetPlacementSystem.h"
#include "Misc/TwinLinkActorEx.h"
#include "TwinLinkWorldViewer.h"

void UTwinLinkFloorViewPanel::SetupTwinLinkFloorView() {
    const auto CityModel = FTwinLinkModule::Get().GetFacilityModel();
    if (CityModel == nullptr) {
        return;
    }
    TArray<UPLATEAUCityObjectGroup*> CityObjects;
    Cast<AActor>(CityModel)->GetComponents<UPLATEAUCityObjectGroup>(CityObjects);
    FString Path = "/PLATEAU-TwinLink/Widgets/WBP_TwinLinkFloorViewElement.WBP_TwinLinkFloorViewElement_C";
    TSubclassOf<class UUserWidget> WidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(*Path)).LoadSynchronous();

    if (WidgetClass == nullptr) {
        return;
    }
    TMap<FString, float> SortFloorKeys;

    for (const auto& CityObject : CityObjects) {
        const auto Obj = CityObject->GetAllRootCityObjects()[0];
        const auto Attribute = Obj.Attributes.AttributeMap.Find("gml:name");
        FString Key = Attribute ? Attribute->StringValue : "Exterior";
        const auto Element = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
        FString Label = Attribute ? Key : TEXT("外観");

        Cast<UTwinLinkFloorViewElement>(Element)->ElementSetup(Label, this);
        LinkComponents.Add(Key, CityObject);
        ElementWidgets.Add(Key, Cast<UUserWidget>(Element));
        const auto Height = Cast<UStaticMeshComponent>(CityObject)->Bounds.GetBox().Min.Z;
        SortFloorKeys.Add(Key, Height);
    }

    //Sort
    for (const auto& SortFloorKey : SortFloorKeys) {
        if (SortFloorKey.Key.Equals("Exterior")) {
            continue;
        }

        int Index = 0;
        for (int i = 0; i < FloorKeys.Num(); i++) {
            if (SortFloorKey.Value > SortFloorKeys[FloorKeys[i]]) {
                break;
            }
            Index++;
        }

        FloorKeys.Insert(SortFloorKey.Key, Index);
    }

    //Sort(Exterior)
    if (SortFloorKeys.Num() > FloorKeys.Num()) {
        FloorKeys.Add("Exterior");
    }

    //ResetChild
    FloorViewScrollBox->ClearChildren();

    //AddChild
    for (const auto& FloorKey : FloorKeys) {
        FloorViewScrollBox->AddChild(Cast<UWidget>(ElementWidgets[FloorKey]));
    }

    SelectedFloorKey = FloorKeys[FloorKeys.Num() - 1];

    //グレーアウトマテリアル準備
    const auto ParentMaterialPath = TEXT("/PLATEAU-TwinLink/Materials/M_FloorGrayedOut");
    if (GrayedOutMaterial == nullptr) {
        GrayedOutMaterial = UMaterialInstanceDynamic::Create(Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, ParentMaterialPath)), this);
    }
    //ブランクマテリアル準備

    const auto BlankParentMaterialPath = TEXT("/PLATEAU-TwinLink/Materials/M_OverlayBlank");
    if (OverlayBlankMaterial == nullptr) {
        OverlayBlankMaterial = UMaterialInstanceDynamic::Create(Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, BlankParentMaterialPath)), this);
    }

    FloorViewChangeKey("Exterior");
}

void UTwinLinkFloorViewPanel::SetupTwinLinkFloorViewWithSwitcher(UTwinLinkFloorSwitcher* Switcher) {
    FloorSwitcher = Switcher;

    if (FloorKeys.IsEmpty()) {
        return;
    }

    SelectedFloorKey = FloorKeys[FloorKeys.Num() - 1];

    FloorViewChangeKey(SelectedFloorKey);

    FloorSwitcher->TwinLinkFloorSwitcherSetup(this);
}

void UTwinLinkFloorViewPanel::FinalizeTwinLinkFloorView() {
    if (ElementWidgets.Find("Exterior") != nullptr) {
        FloorViewChange(ElementWidgets["Exterior"]);
    }
}

void UTwinLinkFloorViewPanel::FloorViewChange(TObjectPtr<UUserWidget> Element) {
    const auto Key = ElementWidgets.FindKey(Element);
    bool IsVisible = false;

    if (Key == nullptr) {
        return;
    }

    if (LinkComponents.Find("Exterior") != nullptr) {
        LinkComponents["Exterior"]->SetVisibility(Key->Equals("Exterior"));
        LinkComponents["Exterior"]->SetCollisionResponseToChannel(ECC_Visibility, Key->Equals("Exterior") ? ECR_Block : ECR_Ignore);
    }

    for (const auto& FloorKey : FloorKeys) {
        if (FloorKey.Equals("Exterior")) {
            continue;
        }

        if (FloorKey.Equals(*Key)) {
            IsVisible = true;
        }

        LinkComponents[*FloorKey]->SetVisibility(IsVisible);
        LinkComponents[*FloorKey]->SetCollisionResponseToChannel(ECC_Visibility, IsVisible ? ECR_Block : ECR_Ignore);
        LinkComponents[*FloorKey]->SetOverlayMaterial(IsVisible ? FloorKey.Equals(*Key) ? OverlayBlankMaterial : GrayedOutMaterial : OverlayBlankMaterial);
    }

    // フロア情報システムに選択状況を保持する　　（フロア情報の設計的に対応できなかったのこの形で実装）
    const auto FloorInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(FloorInfoSys.IsValid());
    FloorInfoSys->SelectedFloor(
        LinkComponents["Exterior"]->GetName(),
        LinkComponents[*Key]->GetName(), *Key);

    // 階層切替が行われたことをブループリントに通知する
    // (本当はUTwinLinkFloorInfoSystemを経由しないで実装したかったが利用先の設計的に受け取れないことが分かったので妥協)
    OnChangedFloorView();
    const auto Arg = LinkComponents[*Key];
    OnChangeFloorDelegate.Broadcast(Arg.Get());

    //選択解除
    if (!SelectedFloorKey.IsEmpty()) {
        Cast<UTwinLinkFloorViewElement>(ElementWidgets[SelectedFloorKey])->OnDeselect();
    }

    SelectedFloorKey = *Key;

    if (FloorSwitcher != nullptr) {
        FloorSwitcher->FloorSwitch();
    }

    //非表示階のアセットは非表示にする

    auto AssetPlacementSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkAssetPlacementSystem>();
    check(AssetPlacementSys.IsValid());

    TArray<TObjectPtr<AActor>> Assets;
    AssetPlacementSys.Get()->GetAssetPlacementActors(Assets);

    for (const auto& Asset : Assets) {
        if (Asset == nullptr) {
            continue;
        }
        Asset->SetActorHiddenInGame(false);
    }

    for (const auto& Component : LinkComponents) {
        if (Component.Key.Equals("Exterior")) {
            continue;
        }
        if (Cast<UStaticMeshComponent>(Component.Value)->IsVisible()) {
            continue;
        }
        for (const auto& Asset : Assets) {
            if (Cast<UStaticMeshComponent>(Component.Value)->Bounds.GetBox().IsInside(Asset->GetActorLocation())) {
                Asset->SetActorHiddenInGame(true);
            }
        }
    }

    //階層にフォーカス

    const auto WorldViewer = TwinLinkActorEx::FindFirstActorInWorld<ATwinLinkWorldViewer>(GetWorld());
    const auto Box = Cast<UStaticMeshComponent>(LinkComponents[*Key])->Bounds.GetBox();
    const auto Distance = FMath::Abs(Box.Max.Z + 10000.0f - Box.GetCenter().Z);
    const auto Dir = FVector(WorldViewer->GetActorForwardVector().X, WorldViewer->GetActorForwardVector().Y, 0.0f).GetSafeNormal();

    FVector V0 = Box.GetCenter();
    FVector V1 = V0 + (-Dir * Distance);
    FVector V2 = FVector(V0.X, V0.Y, V0.Z + Distance);

    //対象からのベクトル
    const auto V01 = V1 - V0;
    const auto V02 = V2 - V0;

    //軸を求める
    auto RotationAxis = FVector::CrossProduct(V01, V02).GetSafeNormal();

    //回転
    const auto V2Primed = FQuat(RotationAxis, FMath::DegreesToRadians(60.0f)) * V01;

    //位置を求める
    const auto v3PrimedFinal = V0 + V2Primed;

    //WorldViewerを移動（対象に向く）
    WorldViewer->SetLocationLookAt(v3PrimedFinal, V0, 0.5f);
}

void UTwinLinkFloorViewPanel::FloorViewChangeKey(const FString& Key) {
    FloorViewChange(ElementWidgets[Key]);
    Cast<UTwinLinkFloorViewElement>(ElementWidgets[Key])->OnSelect();
    Cast<UScrollBox>(FloorViewScrollBox)->ScrollWidgetIntoView(ElementWidgets[Key]);
}