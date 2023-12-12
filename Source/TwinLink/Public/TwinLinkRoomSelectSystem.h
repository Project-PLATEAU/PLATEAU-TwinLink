// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"

#include "TwinLinkRoomSelectSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSelectRoomEvent);

class UProceduralMeshComponent;

/**
 *
 */
UCLASS()
class TWINLINK_API UTwinLinkRoomSelectSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()

public:
    UTwinLinkRoomSelectSystem();
    virtual ~UTwinLinkRoomSelectSystem() override;

    UFUNCTION(BlueprintCallable)
    void UpdateSelection(const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable)
    void UpdateSelectionByUV(class UStaticMeshComponent* Component, const FVector2D& UV);

    UFUNCTION(BlueprintCallable)
    void Focus();

    UFUNCTION(BlueprintCallable)
    UStaticMeshComponent* GetSelectedMesh() const {
        if (SelectedMesh == nullptr || !SelectedMesh.IsValid())
            return nullptr;
        return SelectedMesh.Get();
    }

    UFUNCTION(BlueprintCallable)
    FVector2D GetSelectedUV() const {
        return SelectedUV;
    }

    UFUNCTION(BlueprintCallable)
    FString GetSelectedGroupID() const {
        if (SelectedMesh == nullptr || !SelectedMesh.IsValid())
            return "";
        return SelectedMesh.Get()->GetName();
    }


    UFUNCTION(BlueprintCallable)
    FString GetSelectedRoomID() const;

    UFUNCTION(BlueprintCallable)
    const TArray<FVector>& GetSelectedMeshPositions() const {
        return SelectedMeshPositions;
    }

    UFUNCTION(BlueprintCallable)
    const TArray<int>& GetSelectedMeshTriangles() const {
        return SelectedMeshTriangles;
    }

    UFUNCTION(BlueprintCallable)
    const TArray<FVector2D>& GetSelectedMeshUVs() const {
        return SelectedMeshUVs;
    }

    UFUNCTION(BlueprintCallable)
    FVector GetSelectedMeshCenter() const {
        if (SelectedMeshPositions.Num() == 0)
            return FVector::Zero();

        // 頂点の重心を中心として出す
        FVector Total = FVector::Zero();
        for (const auto Position : SelectedMeshPositions) {
            Total += Position;
        }
        return Total / SelectedMeshPositions.Num();
    }

    UFUNCTION(BlueprintCallable)
    bool IsRoomSelected() const {
        return SelectedMesh != nullptr && SelectedMesh.IsValid();
    }

    UFUNCTION(BlueprintCallable)
    void ClearSelection() {
        SelectedMesh = nullptr;
        SelectRoomEvent.Broadcast();
    }

    UFUNCTION(BlueprintCallable)
    static FVector2D FindCollisionUV(const FHitResult& HitResult);

    FSelectRoomEvent& OnSelectRoom() { return SelectRoomEvent; }

private:
    UPROPERTY(BlueprintAssignable)
    FSelectRoomEvent SelectRoomEvent;

    TWeakObjectPtr<UStaticMeshComponent> SelectedMesh;
    FVector2D SelectedUV;
    TArray<FVector> SelectedMeshPositions;
    TArray<int> SelectedMeshTriangles;
    TArray<FVector2D> SelectedMeshUVs;
    FString RoomID;
};
