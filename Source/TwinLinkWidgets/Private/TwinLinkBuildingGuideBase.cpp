// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkBuildingGuideBase.h"
#include "TwinLinkCommon.h"

#include "TwinLinkWorldViewer.h"
#include "TwinLinkFloorInfoSystem.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLink.h"
#include "PLATEAUInstancedCityModel.h"

void UTwinLinkBuildingGuideBase::NativeConstruct() {
    UTwinLinkTabContentBase::NativeConstruct();
    Setup();
}

void UTwinLinkBuildingGuideBase::NativeDestruct() {
    Finalize();
    UTwinLinkTabContentBase::NativeDestruct();
}

bool UTwinLinkBuildingGuideBase::CheckIsSelectedGrpID(const FString& InGrpID) {
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    return InfoSys->GetGrpIDBySelectedFloor().Equals(InGrpID, ESearchCase::Type::CaseSensitive);
}

void UTwinLinkBuildingGuideBase::OnActivated() {
    UpdateFacilityName();
    //ATwinLinkWorldViewer::ChangeTwinLinkViewMode(GetWorld(), ETwinLinkViewMode::FreeAutoView);
}

void UTwinLinkBuildingGuideBase::Setup() {
    UpdateFacilityName();
}

void UTwinLinkBuildingGuideBase::Finalize() {
}

void UTwinLinkBuildingGuideBase::UpdateFacilityName() {
    const auto CityModel = FTwinLinkModule::Get().GetFacilityModel();
    if (CityModel == nullptr) {
        return;
    }

    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(InfoSys.IsValid());

    const auto FacilityInfoCollection = InfoSys.Get()->GetFacilityInfoCollectionAsMap();

    TArray<UPLATEAUCityObjectGroup*> CityObjects;
    CityModel->GetComponents<UPLATEAUCityObjectGroup>(CityObjects);
    for (int i = 0; i < CityObjects.Num(); i++) {
        const auto Ret = InfoSys.Get()->FindFacilityInfoByFeatureId(CityObjects[i]->GetName());
        if (Ret != nullptr) {
            FacilityName->SetText(FText::FromString(Ret->GetName()));
            OnSetupIcon(Ret->GetCategory());
        }
        break;
    }
}