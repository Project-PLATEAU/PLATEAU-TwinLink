// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkBaseGizmo.h"
#include "TwinLinkTranslationGizmo.generated.h"

/**
 *
 */
UCLASS()
class TWINLINKGIZMO_API ATwinLinkTranslationGizmo : public ATwinLinkBaseGizmo {
    GENERATED_BODY()

public:

    ATwinLinkTranslationGizmo();

    virtual ETwinLinkTransformationType GetGizmoType() const final { return ETwinLinkTransformationType::TT_Translation; }

    virtual FTransform GetDeltaTransform(const FVector& LookingVector
        , const FVector& RayStartPoint
        , const FVector& RayEndPoint, ETwinLinkTransformationDomain Domain) override;

    // Returns a Snapped Transform based on how much has been accumulated, the Delta Transform and Snapping Value
    virtual FTransform GetSnappedTransform(FTransform& outCurrentAccumulatedTransform
        , const FTransform& DeltaTransform
        , ETwinLinkTransformationDomain Domain
        , float SnappingValue) const override;

protected:

    // The Hit Box for the XY-Plane Translation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
        class UBoxComponent* XY_PlaneBox;

    // The Hit Box for the YZ-Plane Translation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
        class UBoxComponent* YZ_PlaneBox;

    // The Hit Box for the  XZ-Plane Translation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
        class UBoxComponent* XZ_PlaneBox;

    // The Hit Box for the  XYZ Free Translation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
        class USphereComponent* XYZ_Sphere;
};

