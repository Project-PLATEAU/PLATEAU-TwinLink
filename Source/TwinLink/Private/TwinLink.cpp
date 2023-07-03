// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLink.h"

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
    bAdminMode = bEnabled;
}

bool FTwinLinkModule::IsAdminModeEnabled() const {
    return bAdminMode;
}

void UTwinLinkBlueprintLibrary::SetAdminMode(const bool bEnabled) {
    FTwinLinkModule::Get().SetAdminMode(bEnabled);
}

bool UTwinLinkBlueprintLibrary::IsAdminModeEnabled() {
    return FTwinLinkModule::Get().IsAdminModeEnabled();
}

IMPLEMENT_MODULE(FTwinLinkModule, TwinLink)