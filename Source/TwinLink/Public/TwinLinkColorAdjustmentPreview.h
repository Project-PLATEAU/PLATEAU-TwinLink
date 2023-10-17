// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TwinLinkColorAdjustmentPreview.generated.h"

UCLASS()
class TWINLINK_API ATwinLinkColorAdjustmentPreview : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATwinLinkColorAdjustmentPreview();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinLink")
        TObjectPtr<UStaticMeshComponent> PreviewTarget;
};
