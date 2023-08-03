// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkBaseGizmo.h"
#include "TwinLinkScaleGizmo.generated.h"

/**
 *
 */
UCLASS()
class TWINLINKGIZMO_API ATwinLinkScaleGizmo : public ATwinLinkBaseGizmo {
    GENERATED_BODY()

public:

    ATwinLinkScaleGizmo();

    virtual ETwinLinkTransformationType GetGizmoType() const final { return ETwinLinkTransformationType::TT_Scale; }

    virtual void UpdateGizmoSpace(ETwinLinkSpaceType SpaceType);

    virtual FTransform GetDeltaTransform(const FVector& LookingVector
        , const FVector& RayStartPoint
        , const FVector& RayEndPoint
        , ETwinLinkTransformationDomain Domain) override;

    // Returns a Snapped Transform based on how much has been accumulated, the Delta Transform and Snapping Value
    virtual FTransform GetSnappedTransform(FTransform& outCurrentAccumulatedTransform
        , const FTransform& DeltaTransform
        , ETwinLinkTransformationDomain Domain
        , float SnappingValue) const override;

    virtual FTransform GetSnappedTransformPerComponent(const FTransform& OldComponentTransform
        , const FTransform& NewComponentTransform
        , ETwinLinkTransformationDomain Domain
        , float SnappingValue) const override;

protected:

    //To see how much an Unreal Unit affects Scaling (e.g. how powerful the mouse scales the object!)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gizmo")
        float ScalingFactor;

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
