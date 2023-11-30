// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLink.h"

#include "Misc/TwinLinkPLATEAUCityModelEx.h"
#include "Interfaces/IPluginManager.h"
void FTwinLinkModule::StartupModule() {
    bAdminMode = false;
}

void FTwinLinkModule::ShutdownModule() {}

FString FTwinLinkModule::GetContentDir() {
    return IPluginManager::Get()
        .FindPlugin(TEXT("PLATEAU-TwinLink"))
        ->GetContentDir();
}

void FTwinLinkModule::SetAdminMode(const bool bEnabled) {
    const auto IsSame = bAdminMode == bEnabled;
    bAdminMode = bEnabled;

    if (IsSame) {
        return;
    }

    if (bEnabled) {
        if (OnActiveAdminMode.IsBound()) {
            OnActiveAdminMode.Broadcast();
        }
    }
    else {
        if (OnInactiveAdminMode.IsBound()) {
            OnInactiveAdminMode.Broadcast();
        }
    }
}

bool FTwinLinkModule::IsAdminModeEnabled() const {
    return bAdminMode;
}

TWeakObjectPtr<UPLATEAUCityObjectGroup> FTwinLinkModule::GetFacilityModelBuildingComponent() const
{
    const auto ModelActor = Cast<APLATEAUInstancedCityModel>(GetFacilityModel());
    if (!ModelActor)
        return nullptr;

    // 最初に見つかったBuilding建築物を返す
    const auto Scanner = TwinLinkPLATEAUInstancedCityModelScanner(ModelActor);
    const auto It = std::find_if(Scanner.begin(), Scanner.end(), [](const FTwinLinkCityObjectGroupModel& Item) {
        return Item.MeshType == FTwinLinkFindCityModelMeshType::Bldg;
        });
    if (It == Scanner.end())
        return nullptr;

    return (*It).CityObjectGroup;
}

void UTwinLinkBlueprintLibrary::SetAdminMode(const bool bEnabled) {
    FTwinLinkModule::Get().SetAdminMode(bEnabled);
}

bool UTwinLinkBlueprintLibrary::IsAdminModeEnabled() {
    return FTwinLinkModule::Get().IsAdminModeEnabled();
}

void UTwinLinkBlueprintLibrary::TwinLinkSetFacilityModel(AActor* Model) {
    FTwinLinkModule::Get().SetFacilityModel(Model);
}

AActor* UTwinLinkBlueprintLibrary::TwinLinkGetFacilityModel() {
    return FTwinLinkModule::Get().GetFacilityModel();
}

void UTwinLinkBlueprintLibrary::TwinLinkSetCityModel(AActor* Model) {
    FTwinLinkModule::Get().SetCityModel(Model);
}

AActor* UTwinLinkBlueprintLibrary::TwinLinkGetCityModel() {
    return FTwinLinkModule::Get().GetCityModel();
}

ESlateVisibility UTwinLinkBlueprintLibrary::AsSlateVisibility(bool visibility)
{
    return visibility ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

IMPLEMENT_MODULE(FTwinLinkModule, TwinLink)