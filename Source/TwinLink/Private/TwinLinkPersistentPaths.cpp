// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkPersistentPaths.h"
#include "Interfaces/IPluginManager.h"

FString TwinLinkPersistentPaths::CreateViewPointFilePath(const FString& FileName) {
    return GetProjSaveDir() + FileName;
}

FString TwinLinkPersistentPaths::CreateAssetPlacementFilePath(const FString& FileName) {
    return GetProjSaveDir() + FileName;
}

FString TwinLinkPersistentPaths::CreateFacilityInfoFilePath(const FString& FileName) {
    return GetProjSaveDir() + FileName;
}

FString TwinLinkPersistentPaths::CreateFacilityImagePath(const FString& FileName) {
    return FString::Printf(TEXT("%s/%s/%s"), *GetProjSaveDir(), TEXT("FacilityImage"), *FileName);
}

FString TwinLinkPersistentPaths::CreateFloorInfoFolderPath() {
    return FString::Printf(TEXT("%s%s"), *GetProjSaveDir(), TEXT("FloorInfo"));
}

FString TwinLinkPersistentPaths::CreateFloorInfoFolderPath(const FString& SubFolderName) {
    return FString::Printf(TEXT("%s%s/%s"), *GetProjSaveDir(), TEXT("FloorInfo"), *SubFolderName);
}

FString TwinLinkPersistentPaths::CreateFloorInfoFilePath(const FString& SubFolderName, const FString& FileName) {
    return FString::Printf(TEXT("%s%s/%s/%s"), *GetProjSaveDir(), TEXT("FloorInfo"), *SubFolderName, *FileName);
}

FString TwinLinkPersistentPaths::GetProjSaveDir() {
    return FPaths::ProjectSavedDir() + TEXT("TwinLink/");
}
