// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.


#include "Gizmos/TwinLinkBaseGizmo.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

// Sets default values
ATwinLinkBaseGizmo::ATwinLinkBaseGizmo() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    ScalingScene = CreateDefaultSubobject<USceneComponent>(TEXT("ScalingScene"));
    ScalingScene->SetupAttachment(RootScene);

    X_AxisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("X Axis Box"));
    Y_AxisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Y Axis Box"));
    Z_AxisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Z Axis Box"));

    X_AxisBox->SetupAttachment(ScalingScene);
    Y_AxisBox->SetupAttachment(ScalingScene);
    Z_AxisBox->SetupAttachment(ScalingScene);

    RegisterDomainComponent(X_AxisBox, ETwinLinkTransformationDomain::TD_X_Axis);
    RegisterDomainComponent(Y_AxisBox, ETwinLinkTransformationDomain::TD_Y_Axis);
    RegisterDomainComponent(Z_AxisBox, ETwinLinkTransformationDomain::TD_Z_Axis);

    GizmoSceneScaleFactor = 0.1f;
    CameraArcRadius = 150.f;

    PreviousRayStartPoint = FVector::ZeroVector;
    PreviousRayEndPoint = FVector::ZeroVector;

    bTransformInProgress = false;
    bIsPrevRayValid = false;
}

void ATwinLinkBaseGizmo::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    //ToDo: There seems to be an issue where the Root Scene doesn't Attach properly on the first 'go' on Unreal 4.26
    if (RootScene) {
        RootScene->AttachToComponent(RootScene->GetAttachParent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    }
}

void ATwinLinkBaseGizmo::UpdateGizmoSpace(ETwinLinkSpaceType SpaceType) {
    switch (SpaceType) {
    case ETwinLinkSpaceType::ST_Local:
        SetActorRelativeRotation(FQuat(EForceInit::ForceInit));
        break;
    case ETwinLinkSpaceType::ST_World:
        SetActorRotation(FQuat(EForceInit::ForceInit), ETeleportType::TeleportPhysics);
        break;
    }
}

//Base Gizmo does not affect anything and returns No Delta Transform.
// This func is overriden by each Transform Gizmo

FTransform ATwinLinkBaseGizmo::GetDeltaTransform(const FVector& LookingVector
    , const FVector& RayStartPoint, const FVector& RayEndPoint
    , ETwinLinkTransformationDomain Domain) {
    FTransform deltaTransform;
    deltaTransform.SetScale3D(FVector::ZeroVector);
    return deltaTransform;
}

void ATwinLinkBaseGizmo::ScaleGizmoScene(const FVector& ReferenceLocation, const FVector& ReferenceLookDirection, float FieldOfView) {
    FVector Scale = CalculateGizmoSceneScale(ReferenceLocation, ReferenceLookDirection, FieldOfView);
    //UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Scale: %s"), *Scale.ToString());
    if (ScalingScene)
        ScalingScene->SetWorldScale3D(Scale);
}

FTransform ATwinLinkBaseGizmo::GetSnappedTransform(FTransform& outCurrentAccumulatedTransform
    , const FTransform& DeltaTransform
    , ETwinLinkTransformationDomain Domain
    , float SnappingValue) const {
    return DeltaTransform;
}

ETwinLinkTransformationDomain ATwinLinkBaseGizmo::GetTransformationDomain(USceneComponent* ComponentHit) const {
    if (!ComponentHit) return ETwinLinkTransformationDomain::TD_None;

    if (UShapeComponent* ShapeComponent = Cast<UShapeComponent>(ComponentHit)) {
        if (const ETwinLinkTransformationDomain* pDomain = DomainMap.Find(ShapeComponent))
            return *pDomain;
    }
    else
        UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Failed to Get Domain! Component Hit is not a Shape Component. %s"), *ComponentHit->GetName());

    return ETwinLinkTransformationDomain::TD_None;
}

FVector ATwinLinkBaseGizmo::CalculateGizmoSceneScale(const FVector& ReferenceLocation, const FVector& ReferenceLookDirection, float FieldOfView) {
    FVector deltaLocation = (GetActorLocation() - ReferenceLocation);
    float distance = deltaLocation.ProjectOnTo(ReferenceLookDirection).Size();
    float scaleView = (distance * FMath::Sin(FMath::DegreesToRadians(FieldOfView))) / CameraArcRadius;
    scaleView *= GizmoSceneScaleFactor;
    return FVector(scaleView);
}

bool ATwinLinkBaseGizmo::AreRaysValid() const {
    return bIsPrevRayValid;
}

void ATwinLinkBaseGizmo::UpdateRays(const FVector& RayStart, const FVector& RayEnd) {
    PreviousRayStartPoint = RayStart;
    PreviousRayEndPoint = RayEnd;
    bIsPrevRayValid = true;
}

void ATwinLinkBaseGizmo::RegisterDomainComponent(USceneComponent* Component
    , ETwinLinkTransformationDomain Domain) {
    if (!Component) return;

    if (UShapeComponent* ShapeComponent = Cast<UShapeComponent>(Component))
        DomainMap.Add(ShapeComponent, Domain);
    else
        UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Failed to Register Component! Component is not a Shape Component %s"), *Component->GetName());
}

void ATwinLinkBaseGizmo::SetTransformProgressState(bool bInProgress
    , ETwinLinkTransformationDomain CurrentDomain) {
    if (bInProgress != bTransformInProgress) {
        bIsPrevRayValid = false; //set this so that we don't get an invalid delta value
        bTransformInProgress = bInProgress;
        OnGizmoStateChange.Broadcast(GetGizmoType(), bTransformInProgress, CurrentDomain);
    }
}
