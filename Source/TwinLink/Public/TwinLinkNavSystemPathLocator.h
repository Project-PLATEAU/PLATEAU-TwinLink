// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemPathLocator.generated.h"

UCLASS()
class TWINLINK_API ATwinLinkNavSystemPathLocator : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathLocator();
protected:
    virtual void BeginPlay() override;
public:
    virtual void Tick(float DeltaSeconds) override;
};
