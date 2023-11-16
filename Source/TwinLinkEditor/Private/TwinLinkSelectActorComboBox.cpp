// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkSelectActorComboBox.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "PLATEAUInstancedCityModel.h"

UTwinLinkSelectActorComboBox::UTwinLinkSelectActorComboBox(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
}

void UTwinLinkSelectActorComboBox::TwinLinkSelectActorComboBoxGenerate() {
    ModelActor = nullptr;

    TSharedRef<SObjectPropertyEntryBox> EntryBox = SNew(SObjectPropertyEntryBox)
        .AllowedClass(APLATEAUInstancedCityModel::StaticClass())
        .ObjectPath_Lambda([this]()->FString const {
        return ModelActor ? ModelActor->GetPathName() : TEXT("");
            })
        .OnObjectChanged_Lambda([this](const FAssetData& assetData) {
                ModelActor = Cast<UObject>(assetData.GetAsset());
            });
            SetContent(EntryBox);
}