// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "TwinLinkSpatialID.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TwinLinkSpatialIDEx.generated.h"

UCLASS()
class TWINLINK_API UTwinLinkSpatialIDEx : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure)
        static int GetX(const FTwinLinkSpatialID& Id) {
        return Id.GetX();
    }
    UFUNCTION(BlueprintPure)
        static int GetY(const FTwinLinkSpatialID& Id) {
        return Id.GetY();
    }
    UFUNCTION(BlueprintPure)
        static int GetZoom(const FTwinLinkSpatialID& Id) {
        return Id.GetZ();
    }
    UFUNCTION(BlueprintPure)
        static int GetAltitude(const FTwinLinkSpatialID& Id) {
        return Id.GetF();
    }
    UFUNCTION(BlueprintPure)
        static bool IsValidAltitude(const FTwinLinkSpatialID& Id) {
        return Id.IsValidAltitude();
    }
    UFUNCTION(BlueprintPure)
        static void BreakParam(const FTwinLinkSpatialID& Id, int& Zoom, int& X, int& Y, bool& bIsValidAttitude, int& Attitude)
    {
        Zoom = Id.GetZ();
        X = Id.GetX();
        Y = Id.GetY();
        bIsValidAttitude = Id.IsValidAltitude();
        Attitude = Id.GetF();
    }
};

