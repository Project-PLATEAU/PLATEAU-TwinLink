// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "EnumT.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"
#include "Camera/CameraActor.h"
#include "TwinLinkNavSystemCamera.generated.h"


UCLASS()
class TWINLINK_API ATwinLinkNavSystemCamera : public ACameraActor {
    GENERATED_BODY()
public:
    ATwinLinkNavSystemCamera();
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable)
        void SetTargetLookAt(const FVector& Location, const FVector& LookAt, bool bForce = false);

private:
    void SetCameraLookAt(const FVector& Location, const FVector& LookAt);
private:
    struct Info
    {
        FVector Location;
        FVector LookAt;
    };
    std::optional<Info> NowInfo;
    std::optional<Info> TargetInfo;
};