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
}

void UTwinLinkFloorViewPanel::FloorViewChangeKey(const FString& Key) {
    FloorViewChange(ElementWidgets[Key]);
    Cast<UTwinLinkFloorViewElement>(ElementWidgets[Key])->OnSelect();
    Cast<UScrollBox>(FloorViewScrollBox)->ScrollWidgetIntoView(ElementWidgets[Key]);
}