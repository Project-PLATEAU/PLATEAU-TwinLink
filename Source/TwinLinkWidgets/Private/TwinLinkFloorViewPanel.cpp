// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkFloorViewPanel.h"
#include "TwinLinkFloorViewElement.h"
#include "Components/PanelWidget.h"
#include "TwinLink.h"

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
    TArray<FString> SortFloorKeys;

    for (const auto& CityObject : CityObjects) {
        const auto Obj = CityObject->GetAllRootCityObjects()[0];
        const auto Attribute = Obj.Attributes.AttributeMap.Find("gml:name");
        FString Key = Attribute ? Attribute->StringValue : "Exterior";
        const auto Element = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
        FString Label = Attribute ? Key.Left(Key.Len() - 1) : TEXT("外観");

        Cast<UTwinLinkFloorViewElement>(Element)->ElementSetup(Label, this);
        LinkComponents.Add(Key, CityObject);
        ElementWidgets.Add(Key, Cast<UUserWidget>(Element));
        SortFloorKeys.Add(Key);
    }

    //Sort
    for (const auto& SortFloorKey : SortFloorKeys) {
        if (SortFloorKey.Equals("Exterior")) {
            continue;
        }

        int Index = 0;
        for (int i = 0; i < FloorKeys.Num(); i++) {
            const auto SortKey = SortFloorKey.Left(SortFloorKey.Len() - 2);
            const auto Key = FloorKeys[i].Left(FloorKeys[i].Len() - 2);
            if (FCString::Atoi(*SortKey) < FCString::Atoi(*Key)) {
                break;
            }
            Index++;
        }

        FloorKeys.Insert(SortFloorKey, Index);
    }

    //Sort(Exterior)
    if (SortFloorKeys.Num() > FloorKeys.Num()) {
        FloorKeys.Insert("Exterior", 0);
    }

    //AddChild
    for (const auto& FloorKey : FloorKeys) {
        FloorViewScrollBox->AddChild(Cast<UWidget>(ElementWidgets[FloorKey]));
    }
}

void UTwinLinkFloorViewPanel::FinalizeTwinLinkFloorView() {
    if (ElementWidgets.Find("Exterior") != nullptr) {
        FloorViewChange(ElementWidgets["Exterior"]);
    }
}

void UTwinLinkFloorViewPanel::FloorViewChange(TObjectPtr<UUserWidget> Element) {
    const auto Key = ElementWidgets.FindKey(Element);
    bool IsVisible = true;

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
        LinkComponents[*FloorKey]->SetVisibility(IsVisible);
        LinkComponents[*FloorKey]->SetCollisionResponseToChannel(ECC_Visibility, IsVisible ? ECR_Block : ECR_Ignore);

        if (FloorKey.Equals(*Key)) {
            IsVisible = false;
        }
    }
}