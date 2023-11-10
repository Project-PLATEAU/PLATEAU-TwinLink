// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkPOIPanel.h"
#include "TwinLinkCommon.h"
#include "TwinLinkPOISubSystem.h"
#include "TwinLinkPOIItem.h"
#include "TwinLinkPOIElement.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"

void UTwinLinkPOIPanel::TwinLinkGISDataGetSource(const TArray<FString>& Files) {
    auto POISubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
    check(POISubSystem.IsValid());

    POISubSystem.Get()->ImportShapefile(Files);

    AttributeCombobox->ClearOptions();

    for (const auto& Attribute : POISubSystem.Get()->AttributeList) {
        AttributeCombobox->AddOption(Attribute);
    }

    AttributeCombobox->RefreshOptions();

    AttributeText->SetText(FText::GetEmpty());
}

void UTwinLinkPOIPanel::TwinLinkPOIRegist() {
    auto POISubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
    check(POISubSystem.IsValid());

    const auto RegisteredName = AttributeText->GetText().ToString();

    if (RegisteredName.IsEmpty()) {
        return;
    }

    const auto SelectedIndex = AttributeCombobox->GetSelectedIndex();

    if (SelectedIndex == -1) {
        return;
    }

    //登録済みなら一度削除
    if (POISubSystem.Get()->RegisteredPOIs.Contains(RegisteredName)) {
        for (const auto& Child : RegisteredAttributesScrollbox->GetAllChildren()) {
            const auto Element = Cast<UTwinLinkPOIElement>(Child);
            if (Element->AttributeText->Text.ToString().Equals(RegisteredName)) {
                Element->TwinLinkPOIRemove();
            }
        }
    }

    POISubSystem.Get()->RegisterAttributes(RegisteredName, SelectedIndex);

    POISubSystem.Get()->AssignMaterials(RegisteredName);

    POISubSystem.Get()->AttributeNames.Add(RegisteredName);

    RegistPOI(RegisteredName);

    ShowPOI(RegisteredName);
}

void UTwinLinkPOIPanel::TwinLinkPOIClear() {
    RegisteredAttributesScrollbox->ClearChildren();
}

void UTwinLinkPOIPanel::TwinLinkPOISetActive(bool IsShow) {
    auto POISubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
    check(POISubSystem.IsValid());

    if (IsShow) {
        for (const auto& Attribute : POISubSystem.Get()->EscapeVisibility) {
            for (const auto& Actor : POISubSystem.Get()->POIActors[Attribute.Key]) {
                Actor->SetActorHiddenInGame(Attribute.Value);
            }
        }
        POISubSystem.Get()->EscapeVisibility.Reset();
    }
    else {
        POISubSystem.Get()->EscapeVisibility.Reset();

        for (const auto& Actors : POISubSystem.Get()->POIActors) {
            POISubSystem.Get()->EscapeVisibility.Add(Actors.Key, Actors.Value[0]->IsHidden());
            for (const auto& Actor : Actors.Value) {
                Actor->SetActorHiddenInGame(true);
            }
        }
    }
}

void UTwinLinkPOIPanel::RegistPOI(const FString& RegisteredName) {
    auto POISubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
    check(POISubSystem.IsValid());

    FString Path = "/PLATEAU-TwinLink/Widgets/Analysis/WBP_TwinLinkPOIElement.WBP_TwinLinkPOIElement_C";
    TSubclassOf<class UTwinLinkPOIElement> WidgetClass = TSoftClassPtr<UTwinLinkPOIElement>(FSoftObjectPath(*Path)).LoadSynchronous();

    if (WidgetClass == nullptr) {
        return;
    }

    FColor Color = POISubSystem.Get()->AssignColors[POISubSystem.Get()->AssignColors.Num() - 1];

    const auto Element = Cast<UTwinLinkPOIElement>(CreateWidget<UUserWidget>(GetWorld(), WidgetClass));
    Element->AttributeText->SetText(FText::FromString(RegisteredName));
    Element->AttributeImage->SetColorAndOpacity(Color);
    Element->Color = Color;
    RegisteredAttributesScrollbox->AddChild(Element);
}

void UTwinLinkPOIPanel::ShowPOI(const FString& RegisteredName) {
    auto POISubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
    check(POISubSystem.IsValid());

    FString Path = "/PLATEAU-TwinLink/Blueprints/Widget3D/BP_POIItem.BP_POIItem_C";
    TSubclassOf<class AActor> WidgetClass = TSoftClassPtr<AActor>(FSoftObjectPath(*Path)).LoadSynchronous();

    if (WidgetClass == nullptr) {
        return;
    }

    TArray<TObjectPtr<AActor>> POIActors;

    const auto Attributes = POISubSystem.Get()->RegisteredPOIs[RegisteredName];

    for (const auto& POI : Attributes) {
        const auto POIActor = GetWorld()->SpawnActor<AActor>(WidgetClass);
        POIActor->SetActorLocation(POI.Value);

        const auto WidgetComponent = Cast<UWidgetComponent>(POIActor->GetComponentByClass(UWidgetComponent::StaticClass()));
        auto Item = Cast<UTwinLinkPOIItem>(WidgetComponent->GetUserWidgetObject());

        Item->AttributeText->SetText(FText::FromString(POI.Key));

        const auto MeshComponent = Cast<UStaticMeshComponent>(POIActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

        MeshComponent->SetMaterial(0, POISubSystem.Get()->MaterialCollection[RegisteredName]);

        POIActors.Add(POIActor);
    }

    POISubSystem.Get()->POIActors.Add(RegisteredName, POIActors);
}