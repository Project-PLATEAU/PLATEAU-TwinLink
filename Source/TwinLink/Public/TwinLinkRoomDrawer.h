// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "TwinLinkRoomSelectSystem.h"
#include "Components/SceneComponent.h"
#include "TwinLinkRoomDrawer.generated.h"


class UTwinLinkRoomSelectSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TWINLINK_API UTwinLinkRoomDrawer : public UProceduralMeshComponent {
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable)
    void DrawRoom(UStaticMeshComponent* MeshComponent, const FVector2D& UV);

    UFUNCTION(BlueprintCallable)
    void Clear();
private:
    UTwinLinkRoomSelectSystem* RoomSelectSystem;

    UFUNCTION()
    void HandleSelectRoom();
};
