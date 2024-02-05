// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorActorLib.h"

#include "Kismet/KismetSystemLibrary.h"

#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUCityObjectGroup.h"

bool UTwinLinkEditorActorLib::SetupCityModel(UObject* CityModel) {
    if (CityModel == nullptr) {
        return false;
    }

    APLATEAUInstancedCityModel* CastedCityModel = Cast<APLATEAUInstancedCityModel>(CityModel);
    TArray<UActorComponent*> CityObjectGroups;
    CastedCityModel->GetComponents(UPLATEAUCityObjectGroup::StaticClass(), CityObjectGroups);

    const auto bIsReceivesDecals = false;
    for (auto& ObjGroups : CityObjectGroups) {
        UPLATEAUCityObjectGroup* CastedCityObjectGroup = Cast<UPLATEAUCityObjectGroup>(ObjGroups);
        if (CastedCityObjectGroup == nullptr) {
            continue;
        }
        const auto bIsDem = CastedCityObjectGroup->GetName().StartsWith("Dem_", ESearchCase::IgnoreCase);
        if (bIsDem == false) {
            CastedCityObjectGroup->SetReceivesDecals(false);
        }
    }

    return true;
}
