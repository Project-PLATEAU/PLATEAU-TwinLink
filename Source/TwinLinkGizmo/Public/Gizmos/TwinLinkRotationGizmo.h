// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkBaseGizmo.h"
#include "TwinLinkRotationGizmo.generated.h"

/**
 *
 */
UCLASS()
class TWINLINKGIZMO_API ATwinLinkRotationGizmo : public ATwinLinkBaseGizmo {
    GENERATED_BODY()

public:

    ATwinLinkRotationGizmo();

    virtual ETwinLinkTransformationType GetGizmoType() const final { return ETwinLinkTransformationType::TT_Rotation; }

    // Returns a Snapped Transform based on how much has been accumulated, the Delta Transform and Snapping Value
    virtual FTransform GetSnappedTransform(FTransform& outCurrentAccumulatedTransform
        , const FTransform& DeltaTransform
        , ETwinLinkTransformationDomain Domain
        , float SnappingValue) const override;

protected:

    //Rotation has a special way of Handling the Scene Scaling and that is, that its AXis need to face the Camera as well!
    virtual FVector CalculateGizmoSceneScale(const FVector& ReferenceLocation, const FVector& ReferenceLookDirection
        , float FieldOfView) override;

    virtual FTransform GetDeltaTransform(const FVector& LookingVector
        , const FVector& RayStartPoint
        , const FVector& RayEndPoint
        , ETwinLinkTransformationDomain Domain) override;

private:

    FVector PreviousRotationViewScale;
};
