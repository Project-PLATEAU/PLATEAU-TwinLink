// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorActorLib.h"

#include "Kismet/KismetSystemLibrary.h"

#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUCityObjectGroup.h"

#include "Misc/TwinLinkPLATEAUCityModelEx.h"

bool UTwinLinkEditorActorLib::SetupCityModel(UObject* CityModel) {
    if (CityModel == nullptr) {
        return false;
    }

    APLATEAUInstancedCityModel* CastedCityModel = Cast<APLATEAUInstancedCityModel>(CityModel);
    TArray<UActorComponent*> CityObjectGroups;
    CastedCityModel->GetComponents(UPLATEAUCityObjectGroup::StaticClass(), CityObjectGroups);
    auto type = FTwinLinkPLATEAUCityModelEx::ParseMeshType(CastedCityModel->GetName());
    for (auto& CityObjectGroup : CityObjectGroups) {
        auto Type = FTwinLinkPLATEAUCityModelEx::ParseMeshType(CityObjectGroup->GetName());
        if (Type != FTwinLinkFindCityModelMeshType::Dem) {
            auto a = Cast<UPLATEAUCityObjectGroup>(CityObjectGroup);
            if (a == nullptr)
                continue;
            a->SetReceivesDecals(false);
        }
    }

    return true;
}
