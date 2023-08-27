// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TwinLinkNavSystemPathDrawer.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TWINLINK_API UTwinLinkNavSystemPathDrawer : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UTwinLinkNavSystemPathDrawer();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
};
