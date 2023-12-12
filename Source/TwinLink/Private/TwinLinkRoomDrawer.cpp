// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkRoomDrawer.h"

#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "TwinLinkRoomSelectSystem.h"

void UTwinLinkRoomDrawer::BeginPlay() {
    Super::BeginPlay();

    const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
    const auto GameInstance = Cast<UGameInstance>(Context->OwningGameInstance);
    RoomSelectSystem = GameInstance->GetSubsystem<UTwinLinkRoomSelectSystem>();

    ensure(RoomSelectSystem);

    RoomSelectSystem->OnSelectRoom().AddDynamic(this, &UTwinLinkRoomDrawer::HandleSelectRoom);
}

void UTwinLinkRoomDrawer::DrawRoom(UStaticMeshComponent* MeshComponent, const FVector2D& UV) {
    ensure(RoomSelectSystem);

    const TArray<FVector> ProceduralVertices = RoomSelectSystem->GetSelectedMeshPositions();
    const TArray<int> ProceduralTriangles = RoomSelectSystem->GetSelectedMeshTriangles();
    const TArray<FVector2D> ProceduralUVs = RoomSelectSystem->GetSelectedMeshUVs();

    TArray<FVector> ProceduralNormal;
    TArray<FProcMeshTangent> ProceduralTangents;
    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(ProceduralVertices, ProceduralTriangles, ProceduralUVs, ProceduralNormal, ProceduralTangents);

    const TArray<FColor> ProceduralColors;
    CreateMeshSection(0, ProceduralVertices, ProceduralTriangles, ProceduralNormal, ProceduralUVs, ProceduralColors, ProceduralTangents, false);
    UpdateBounds();
}

void UTwinLinkRoomDrawer::Clear() {
    ClearAllMeshSections();
}

void UTwinLinkRoomDrawer::HandleSelectRoom() {
    ensure(RoomSelectSystem);

    if (RoomSelectSystem->GetSelectedMesh() == nullptr) {
        Clear();
        return;
    }

    DrawRoom(RoomSelectSystem->GetSelectedMesh(), RoomSelectSystem->GetSelectedUV());
}

