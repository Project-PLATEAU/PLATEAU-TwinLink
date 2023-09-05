// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkTransformerComponent.h"

#include "GameFramework/PlayerController.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

/* Gizmos */
#include "Gizmos/TwinLinkBaseGizmo.h"
#include "Gizmos/TwinLinkTranslationGizmo.h"
#include "Gizmos/TwinLinkRotationGizmo.h"
#include "Gizmos/TwinLinkScaleGizmo.h"

/* Interface */
#include "TwinLinkFocusableObject.h"

// Sets default values for this component's properties
UTwinLinkTransformerComponent::UTwinLinkTransformerComponent() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    GizmoPlacement = ETwinLinkGizmoPlacement::GP_OnLastSelection;
    CurrentTransformation = ETwinLinkTransformationType::TT_Translation;
    CurrentDomain = ETwinLinkTransformationDomain::TD_None;
    CurrentSpaceType = ETwinLinkSpaceType::ST_World;
    TranslationGizmoClass = ATwinLinkTranslationGizmo::StaticClass();
    RotationGizmoClass = ATwinLinkRotationGizmo::StaticClass();
    ScaleGizmoClass = ATwinLinkScaleGizmo::StaticClass();

    CloneReplicationCheckFrequency = 0.05f;
    MinimumCloneReplicationTime = 0.01f;

    bResyncSelection = false;
    //bReplicates = false;
    bIgnoreNonReplicatedObjects = false;

    ResetDeltaTransform(AccumulatedDeltaTransform);
    ResetDeltaTransform(NetworkDeltaTransform);

    SetTransformationType(CurrentTransformation);
    SetSpaceType(CurrentSpaceType);

    bTransformUFocusableObjects = true;
    bRotateOnLocalAxis = false;
    bForceMobility = false;
    bToggleSelectedInMultiSelection = true;
    bComponentBased = false;
}

// Called when the game starts
void UTwinLinkTransformerComponent::BeginPlay() {
    Super::BeginPlay();

    // ...
}

// Called every frame
void UTwinLinkTransformerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!Gizmo.IsValid()) return;

    const auto Controller = UGameplayStatics::GetPlayerController(GetOwner(), 0);

    if (APlayerController* PlayerController = Cast< APlayerController>(Controller)) {
        FVector worldLocation, worldDirection;
        if (PlayerController->IsLocalController() && PlayerController->PlayerCameraManager) {
            if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection)) {
                FTransform deltaTransform = UpdateTransform(PlayerController->PlayerCameraManager->GetActorForwardVector()
                    , worldLocation, worldDirection);

                NetworkDeltaTransform = FTransform(
                    deltaTransform.GetRotation() * NetworkDeltaTransform.GetRotation(),
                    deltaTransform.GetLocation() + NetworkDeltaTransform.GetLocation(),
                    deltaTransform.GetScale3D() + NetworkDeltaTransform.GetScale3D());
            }
        }
    }

    //Only consider Local View
    if (APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(GetOwner(), 0)) {
        if (LocalPlayerController->PlayerCameraManager) {
            Gizmo->ScaleGizmoScene(LocalPlayerController->PlayerCameraManager->GetCameraLocation()
                , LocalPlayerController->PlayerCameraManager->GetActorForwardVector()
                , LocalPlayerController->PlayerCameraManager->GetFOVAngle());
        }
    }

    Gizmo->UpdateGizmoSpace(CurrentSpaceType); //ToDo: change when this is called to improve performance when a gizmo is there without doing anything
}

UObject* UTwinLinkTransformerComponent::GetUFocusable(USceneComponent* Component) const {
    if (!Component) return nullptr;
    if (bComponentBased)
        return (Component->Implements<UTwinLinkFocusableObject>()) ? Component : nullptr;
    else if (AActor* ComponentOwner = Component->GetOwner())
        return (ComponentOwner->Implements<UTwinLinkFocusableObject>()) ? ComponentOwner : nullptr;
    return nullptr;
}

void UTwinLinkTransformerComponent::SetTransform(USceneComponent* Component, const FTransform& Transform) {
    if (!Component) return;
    if (UObject* focusableObject = GetUFocusable(Component)) {
        //TODO ITwinLinkFocusableObjectは今のところ使用予定なし
        //ITwinLinkFocusableObject::Execute_OnNewTransformation(focusableObject, this, Component, Transform, bComponentBased);
        if (bTransformUFocusableObjects)
            Component->SetWorldTransform(Transform);
    }
    else
        Component->SetWorldTransform(Transform);
}

void UTwinLinkTransformerComponent::Select(USceneComponent* Component, bool* bImplementsUFocusable) {
    UObject* focusableObject = GetUFocusable(Component);
    if (focusableObject)
        //TODO ITwinLinkFocusableObjectは今のところ使用予定なし
        //ITwinLinkFocusableObject::Execute_Focus(focusableObject, this, Component, bComponentBased);
        if (bImplementsUFocusable)
            *bImplementsUFocusable = !!focusableObject;
}

void UTwinLinkTransformerComponent::Deselect(USceneComponent* Component, bool* bImplementsUFocusable) {
    UObject* focusableObject = GetUFocusable(Component);
    if (focusableObject)
        //TODO ITwinLinkFocusableObjectは今のところ使用予定なし
        //ITwinLinkFocusableObject::Execute_Unfocus(focusableObject, this, Component, bComponentBased);
        if (bImplementsUFocusable)
            *bImplementsUFocusable = !!focusableObject;
}

void UTwinLinkTransformerComponent::FilterHits(TArray<FHitResult>& outHits) {
    //eliminate all outHits that have non-replicated objects
    if (bIgnoreNonReplicatedObjects) {
        for (auto Iter = outHits.CreateIterator(); Iter; ++Iter) {
            //don't remove Gizmos! They do not replicate by default
            if (Cast<ATwinLinkBaseGizmo>(Iter->GetActor()))
                continue;

            if (IsValid(Iter->GetActor()) && Iter->GetActor()->IsSupportedForNetworking()) {
                if (bComponentBased) {
                    if (Iter->Component.IsValid() && Iter->Component->IsSupportedForNetworking())
                        continue; //components - actor owner + themselves need to replicate
                }
                else
                    continue; //actors only consider whether they replicate
            }

            if (IsValid(Iter->GetActor()) && Iter->Component.IsValid()) {
                UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Removing (Actor: %s   ComponentHit:  %s) from hits because it is not supported for networking.")
                    , *Iter->GetActor()->GetName(), *Iter->Component->GetName());
            }

            Iter.RemoveCurrent();
        }
    }
}

void UTwinLinkTransformerComponent::SetSpaceType(ETwinLinkSpaceType Type) {
    CurrentSpaceType = Type;
    SetGizmo();
}

ETwinLinkTransformationDomain UTwinLinkTransformerComponent::GetCurrentDomain(bool& TransformInProgress) const {
    TransformInProgress = (CurrentDomain != ETwinLinkTransformationDomain::TD_None);
    return CurrentDomain;
}

void UTwinLinkTransformerComponent::ClearDomain() {
    //Clear the Accumulated tranform when we stop Transforming
    ResetDeltaTransform(AccumulatedDeltaTransform);
    SetDomain(ETwinLinkTransformationDomain::TD_None);
}

bool UTwinLinkTransformerComponent::GetMouseStartEndPoints(float TraceDistance, FVector& outStartPoint, FVector& outEndPoint) {
    const auto Controller = UGameplayStatics::GetPlayerController(GetOwner(), 0);

    if (APlayerController* PlayerController = Cast< APlayerController>(Controller)) {
        FVector worldLocation, worldDirection;
        if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection)) {
            outStartPoint = worldLocation;
            outEndPoint = worldLocation + (worldDirection * TraceDistance);
            return true;
        }
    }
    return false;
}

UClass* UTwinLinkTransformerComponent::GetGizmoClass(ETwinLinkTransformationType TransformationType) const /* private */
{
    //Assign correct Gizmo Class depending on given Transformation
    switch (CurrentTransformation) {
    case ETwinLinkTransformationType::TT_Translation:   return TranslationGizmoClass;
    case ETwinLinkTransformationType::TT_Rotation:      return RotationGizmoClass;
    case ETwinLinkTransformationType::TT_Scale:         return ScaleGizmoClass;
    default:                                    return nullptr;
    }
}

void UTwinLinkTransformerComponent::ResetDeltaTransform(FTransform& Transform) {
    Transform = FTransform();
    Transform.SetScale3D(FVector::ZeroVector);
}

void UTwinLinkTransformerComponent::SetDomain(ETwinLinkTransformationDomain Domain) {
    CurrentDomain = Domain;

    if (Gizmo.IsValid())
        Gizmo->SetTransformProgressState(CurrentDomain != ETwinLinkTransformationDomain::TD_None
            , CurrentDomain);
}

bool UTwinLinkTransformerComponent::MouseTraceByObjectTypes(float TraceDistance
    , TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
    , TArray<AActor*> IgnoredActors, bool bAppendToList) {
    FVector start, end;
    bool bTraceSuccessful = false;
    if (GetMouseStartEndPoints(TraceDistance, start, end)) {
        bTraceSuccessful = TraceByObjectTypes(start, end, CollisionChannels
            , IgnoredActors, bAppendToList);

        if (!bTraceSuccessful && !bAppendToList)
            ServerDeselectAll(false);
    }
    return bTraceSuccessful;
}

bool UTwinLinkTransformerComponent::MouseTraceByChannel(float TraceDistance
    , TEnumAsByte<ECollisionChannel> TraceChannel, TArray<AActor*> IgnoredActors
    , bool bAppendToList) {
    FVector start, end;
    bool bTraceSuccessful = false;
    if (GetMouseStartEndPoints(TraceDistance, start, end)) {
        bTraceSuccessful = TraceByChannel(start, end, TraceChannel
            , IgnoredActors, bAppendToList);
        if (!bTraceSuccessful && !bAppendToList)
            ServerDeselectAll(false);
    }
    return false;
}

bool UTwinLinkTransformerComponent::MouseTraceByProfile(float TraceDistance
    , const FName& ProfileName
    , TArray<AActor*> IgnoredActors
    , bool bAppendToList) {
    FVector start, end;
    bool bTraceSuccessful = false;
    if (GetMouseStartEndPoints(TraceDistance, start, end)) {
        bTraceSuccessful = TraceByProfile(start, end, ProfileName
            , IgnoredActors, bAppendToList);
        if (!bTraceSuccessful && !bAppendToList)
            ServerDeselectAll(false);
    }
    return bTraceSuccessful;
}

bool UTwinLinkTransformerComponent::TraceByObjectTypes(const FVector& StartLocation
    , const FVector& EndLocation
    , TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
    , TArray<AActor*> IgnoredActors
    , bool bAppendToList) {
    if (UWorld* world = GetWorld()) {
        FCollisionObjectQueryParams CollisionObjectQueryParams;
        FCollisionQueryParams CollisionQueryParams;

        //Add All Given Collisions to the Array
        for (auto& cc : CollisionChannels)
            CollisionObjectQueryParams.AddObjectTypesToQuery(cc);

        CollisionQueryParams.AddIgnoredActors(IgnoredActors);

        TArray<FHitResult> OutHits;
        if (world->LineTraceMultiByObjectType(OutHits, StartLocation, EndLocation
            , CollisionObjectQueryParams, CollisionQueryParams)) {
            FilterHits(OutHits);
            return HandleTracedObjects(OutHits, bAppendToList);
        }
    }
    return false;
}

bool UTwinLinkTransformerComponent::TraceByChannel(const FVector& StartLocation
    , const FVector& EndLocation
    , TEnumAsByte<ECollisionChannel> TraceChannel
    , const TArray<AActor*>& IgnoredActors
    , bool bAppendToList) {
    if (UWorld* world = GetWorld()) {
        FCollisionQueryParams CollisionQueryParams;
        CollisionQueryParams.AddIgnoredActors(IgnoredActors);

        TArray<FHitResult> OutHits;
        if (world->LineTraceMultiByChannel(OutHits, StartLocation, EndLocation
            , TraceChannel, CollisionQueryParams)) {
            FilterHits(OutHits);
            return HandleTracedObjects(OutHits, bAppendToList);
        }
    }
    return false;
}

bool UTwinLinkTransformerComponent::TraceByProfile(const FVector& StartLocation
    , const FVector& EndLocation
    , const FName& ProfileName, TArray<AActor*> IgnoredActors
    , bool bAppendToList) {
    if (UWorld* world = GetWorld()) {
        FCollisionQueryParams CollisionQueryParams;
        CollisionQueryParams.AddIgnoredActors(IgnoredActors);

        TArray<FHitResult> OutHits;
        if (world->LineTraceMultiByProfile(OutHits, StartLocation, EndLocation
            , ProfileName, CollisionQueryParams)) {
            FilterHits(OutHits);
            return HandleTracedObjects(OutHits, bAppendToList);
        }
    }
    return false;
}

FTransform UTwinLinkTransformerComponent::UpdateTransform(const FVector& LookingVector
    , const FVector& RayOrigin
    , const FVector& RayDirection) {
    FTransform deltaTransform;
    deltaTransform.SetScale3D(FVector::ZeroVector);

    if (!Gizmo.IsValid() || CurrentDomain == ETwinLinkTransformationDomain::TD_None)
        return deltaTransform;

    FVector rayEnd = RayOrigin + 1'000'000'00 * RayDirection;

    FTransform calcDeltaTransform = Gizmo->GetDeltaTransform(LookingVector, RayOrigin, rayEnd, CurrentDomain);

    //The delta transform we are actually going to apply (same if there is no Snapping taking place)
    deltaTransform = calcDeltaTransform;

    /* SNAPPING LOGIC */
    bool* snappingEnabled = SnappingEnabled.Find(CurrentTransformation);
    float* snappingValue = SnappingValues.Find(CurrentTransformation);

    if (snappingEnabled && *snappingEnabled && snappingValue)
        deltaTransform = Gizmo->GetSnappedTransform(AccumulatedDeltaTransform
            , calcDeltaTransform, CurrentDomain, *snappingValue);
    //GetSnapped Transform Modifies Accumulated Delta Transform by how much Snapping Occurred

    ApplyDeltaTransform(deltaTransform);
    return deltaTransform;
}

void UTwinLinkTransformerComponent::ApplyDeltaTransform(const FTransform& DeltaTransform) {
    bool* snappingEnabled = SnappingEnabled.Find(CurrentTransformation);
    float* snappingValue = SnappingValues.Find(CurrentTransformation);

    for (auto& sc : SelectedComponents) {
        if (!sc) continue;
        if (bForceMobility || sc->Mobility == EComponentMobility::Type::Movable) {
            const FTransform& componentTransform = sc->GetComponentTransform();

            FQuat deltaRotation = DeltaTransform.GetRotation();

            FVector deltaLocation = componentTransform.GetLocation()
                - Gizmo->GetActorLocation();

            //DeltaScale is Unrotated Scale to Get Local Scale since World Scale is not supported
            FVector deltaScale = componentTransform.GetRotation()
                .UnrotateVector(DeltaTransform.GetScale3D());

            if (false == bRotateOnLocalAxis)
                deltaLocation = deltaRotation.RotateVector(deltaLocation);

            FTransform newTransform(
                deltaRotation * componentTransform.GetRotation(),
                //adding Gizmo Location + prevDeltaLocation
                // (i.e. location from Gizmo to Object after optional Rotating)
                // + deltaTransform Location Offset
                deltaLocation + Gizmo->GetActorLocation() + DeltaTransform.GetLocation(),
                deltaScale + componentTransform.GetScale3D());

            /* SNAPPING LOGIC PER COMPONENT */
            if (snappingEnabled && *snappingEnabled && snappingValue)
                newTransform = Gizmo->GetSnappedTransformPerComponent(componentTransform
                    , newTransform, CurrentDomain, *snappingValue);

            sc->SetMobility(EComponentMobility::Type::Movable);
            SetTransform(sc, newTransform);
        }
        else {
            UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Transform will not affect Component [%s] as it is NOT Moveable!"), *sc->GetName());
        }
    }
}

bool UTwinLinkTransformerComponent::HandleTracedObjects(const TArray<FHitResult>& HitResults
    , bool bAppendToList) {
    //Assign as None just in case we don't hit Any Gizmos
    ClearDomain();

    //Search for our Gizmo (if Valid) First before Selecting any item
    if (Gizmo.IsValid()) {
        for (auto& hitResult : HitResults) {
            if (Gizmo == hitResult.GetActor()) //check if it's OUR gizmo
            {
                //Check which Domain of Gizmo was Hit from the Test
                if (USceneComponent* componentHit = Cast<USceneComponent>(hitResult.Component)) {
                    SetDomain(Gizmo->GetTransformationDomain(componentHit));
                    if (CurrentDomain != ETwinLinkTransformationDomain::TD_None) {
                        Gizmo->SetTransformProgressState(true, CurrentDomain);
                        return true; //finish only if the component actually has a domain, else continue
                    }
                }
            }
        }
    }

    for (auto& hits : HitResults) {
        if (Cast<ATwinLinkBaseGizmo>(hits.GetActor()))
            continue; //ignore other Gizmos.

        if (bComponentBased)
            SelectComponent(Cast<USceneComponent>(hits.GetComponent()), bAppendToList);
        else
            SelectActor(hits.GetActor(), bAppendToList);

        return true; //don't process more!
    }

    return false;
}

void UTwinLinkTransformerComponent::SetComponentBased(bool bIsComponentBased) {
    auto selectedComponents = DeselectAll();
    bComponentBased = bIsComponentBased;
    if (bComponentBased)
        SelectMultipleComponents(selectedComponents, false);
    else {
        TArray<AActor*> actors;
        for (auto& c : selectedComponents)
            actors.Add(c->GetOwner());
        SelectMultipleActors(actors, false);
    }
}

void UTwinLinkTransformerComponent::SetRotateOnLocalAxis(bool bRotateLocalAxis) {
    bRotateOnLocalAxis = bRotateLocalAxis;
}

void UTwinLinkTransformerComponent::SetTransformationType(ETwinLinkTransformationType TransformationType) {
    //Don't continue if these are the same.
    if (CurrentTransformation == TransformationType) return;

    if (TransformationType == ETwinLinkTransformationType::TT_NoTransform) {
        UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Setting Transformation Type to None!"));
    }

    CurrentTransformation = TransformationType;

    //Clear the Accumulated tranform when we have a new Transformation
    ResetDeltaTransform(AccumulatedDeltaTransform);

    UpdateGizmoPlacement();
}

void UTwinLinkTransformerComponent::SetSnappingEnabled(ETwinLinkTransformationType TransformationType, bool bSnappingEnabled) {
    SnappingEnabled.Add(TransformationType, bSnappingEnabled);
}

void UTwinLinkTransformerComponent::SetSnappingValue(ETwinLinkTransformationType TransformationType, float SnappingValue) {
    SnappingValues.Add(TransformationType, SnappingValue);
}

void UTwinLinkTransformerComponent::GetSelectedComponents(TArray<class USceneComponent*>& outComponentList
    , USceneComponent*& outGizmoPlacedComponent) const {
    outComponentList = SelectedComponents;
    if (Gizmo.IsValid())
        outGizmoPlacedComponent = Gizmo->GetParentComponent();
}

TArray<USceneComponent*> UTwinLinkTransformerComponent::GetSelectedComponents() const {
    return SelectedComponents;
}

void UTwinLinkTransformerComponent::CloneSelected(bool bSelectNewClones
    , bool bAppendToList) {
    if (GetOwner()->GetLocalRole() < ROLE_Authority) {
        UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Cloning in a Non-Authority! Please use the Clone RPCs instead"));
    }

    auto CloneComponents = CloneFromList(SelectedComponents);

    if (bSelectNewClones)
        SelectMultipleComponents(CloneComponents, bAppendToList);
}

TArray<class USceneComponent*> UTwinLinkTransformerComponent::CloneFromList(const TArray<USceneComponent*>& ComponentList) {
    TArray<class USceneComponent*> outClones;
    if (bComponentBased) {
        TArray<USceneComponent*> Components;

        for (auto& i : ComponentList) {
            if (i) Components.Add(i);
        }
        outClones = CloneComponents(Components);
    }
    else {
        TArray<AActor*> Actors;
        for (auto& i : ComponentList) {
            if (i)
                Actors.Add(i->GetOwner());
        }
        outClones = CloneActors(Actors);
    }

    if (CurrentDomain != ETwinLinkTransformationDomain::TD_None && Gizmo.IsValid())
        Gizmo->SetTransformProgressState(true, CurrentDomain);

    return outClones;
}

TArray<class USceneComponent*> UTwinLinkTransformerComponent::CloneActors(const TArray<AActor*>& Actors) {
    TArray<class USceneComponent*> outClones;

    UWorld* world = GetWorld();
    if (!world) return outClones;

    TSet<AActor*>   actorsProcessed;
    TArray<AActor*> actorsToSelect;
    for (auto& templateActor : Actors) {
        if (!templateActor) continue;
        bool bAlreadyProcessed;
        actorsProcessed.Add(templateActor, &bAlreadyProcessed);
        if (bAlreadyProcessed) continue;

        FTransform spawnTransform;
        FActorSpawnParameters spawnParams;

        spawnParams.Template = templateActor;
        if (templateActor)
            templateActor->bNetStartup = false;

        if (AActor* actor = world->SpawnActor(templateActor->GetClass()
            , &spawnTransform, spawnParams)) {
            outClones.Add(actor->GetRootComponent());
        }
    }
    return outClones;
}

TArray<class USceneComponent*> UTwinLinkTransformerComponent::CloneComponents(const TArray<class USceneComponent*>& Components) {
    TArray<class USceneComponent*> outClones;
    //TODO クローンは使用予定なし
    //UWorld* world = GetWorld();
    //if (!world) return outClones;

    //TMap<USceneComponent*, USceneComponent*> OcCc; //Original component - Clone component
    //TMap<USceneComponent*, USceneComponent*> CcOp; //Clone component - Original parent

    ////clone components phase
    //for (auto& templateComponent : Components) {
    //    if (!templateComponent) continue;
    //    AActor* owner = templateComponent->GetOwner();
    //    if (!owner) continue;

    //    if (USceneComponent* clone = Cast<USceneComponent>(
    //        StaticDuplicateObject(templateComponent, owner))) {
    //        PostCreateBlueprintComponent(clone);
    //        clone->OnComponentCreated();

    //        clone->RegisterComponent();
    //        clone->SetRelativeTransform(templateComponent->GetRelativeTransform());

    //        outClones.Add(clone);
    //        //Add to these two maps for reparenting in next phase
    //        OcCc.Add(templateComponent, clone); //Original component - Clone component

    //        if (templateComponent == owner->GetRootComponent())
    //            CcOp.Add(clone, owner->GetRootComponent()); //this will cause a loop in the maps, so we must check for this!
    //        else
    //            CcOp.Add(clone, templateComponent->GetAttachParent()); //Clone component - Original parent
    //    }
    //}

    //TArray<USceneComponent*> componentsToSelect;
    ////reparenting phase
    //FAttachmentTransformRules attachmentRule(EAttachmentRule::KeepWorld, false);
    //for (auto& cp : CcOp) {
    //    //original parent
    //    USceneComponent* parent = cp.Value;

    //    AActor* actorOwner = cp.Value->GetOwner();

    //    //find if we cloned the original parent
    //    USceneComponent** cloneParent = OcCc.Find(parent);

    //    if (cloneParent) {
    //        if (*cloneParent != cp.Key) //make sure comp is not its own parent
    //            parent = *cloneParent;
    //    }
    //    else {
    //        //couldn't find its parent, so find the parent of the parent and see if it's in the list.
    //        //repeat until found or root is reached
    //        while (1) {
    //            //if parent is root, then no need to find parents above it. (none)
    //            //attach to original parent, since there are no cloned parents.
    //            if (parent == actorOwner->GetRootComponent()) {
    //                parent = cp.Value;
    //                break;
    //            }

    //            //check if parents have been cloned
    //            cloneParent = OcCc.Find(parent->GetAttachParent());
    //            if (cloneParent) {
    //                //attach to cloned parent if found
    //                parent = *cloneParent;
    //                break;
    //            }
    //            parent = parent->GetAttachParent(); //move up in the hierarchy
    //        }
    //    }

    //    cp.Key->AttachToComponent(parent, attachmentRule);

    //    //Selecting childs and parents can cause weird issues
    //    // so only select the topmost clones (those that do not have cloned parents!)
    //    //only select those that have an "original parent".
    //    //if ((parent == cp.Value
    //    //  || parent == actorOwner->GetRootComponent()))
    //    //  //check if the parent of the cloned is original (means it's topmost)
    //    //  outParents.Add(cp.Key);
    //}

    return outClones;
}

void UTwinLinkTransformerComponent::SelectComponent(class USceneComponent* Component
    , bool bAppendToList) {
    if (!Component) return;

    if (ShouldSelect(Component->GetOwner(), Component)) {
        if (false == bAppendToList)
            DeselectAll();
        AddComponent_Internal(SelectedComponents, Component);
        UpdateGizmoPlacement();
    }
}

void UTwinLinkTransformerComponent::SelectActor(AActor* Actor
    , bool bAppendToList) {
    if (!Actor) return;

    if (ShouldSelect(Actor, Actor->GetRootComponent())) {
        if (false == bAppendToList)
            DeselectAll();
        AddComponent_Internal(SelectedComponents, Actor->GetRootComponent());
        UpdateGizmoPlacement();
    }
}

void UTwinLinkTransformerComponent::SelectMultipleComponents(const TArray<USceneComponent*>& Components
    , bool bAppendToList) {
    bool bValidList = false;

    for (auto& c : Components) {
        if (!c) continue;
        if (!ShouldSelect(c->GetOwner(), c)) continue;

        if (false == bAppendToList) {
            DeselectAll();
            bAppendToList = true;
            //only run once. This is not place outside in case a list is empty or contains only invalid components
        }
        bValidList = true;
        AddComponent_Internal(SelectedComponents, c);
    }

    if (bValidList) UpdateGizmoPlacement();
}

void UTwinLinkTransformerComponent::SelectMultipleActors(const TArray<AActor*>& Actors
    , bool bAppendToList) {
    bool bValidList = false;
    for (auto& a : Actors) {
        if (!a) continue;
        if (!ShouldSelect(a, a->GetRootComponent())) continue;

        if (false == bAppendToList) {
            DeselectAll();
            bAppendToList = true;
            //only run once. This is not place outside in case a list is empty or contains only invalid components
        }

        bValidList = true;
        AddComponent_Internal(SelectedComponents, a->GetRootComponent());
    }
    if (bValidList) UpdateGizmoPlacement();
}

void UTwinLinkTransformerComponent::DeselectComponent(USceneComponent* Component) {
    if (!Component) return;
    DeselectComponent_Internal(SelectedComponents, Component);
    UpdateGizmoPlacement();
}

void UTwinLinkTransformerComponent::DeselectActor(AActor* Actor) {
    if (Actor)
        DeselectComponent(Actor->GetRootComponent());
}

TArray<USceneComponent*> UTwinLinkTransformerComponent::DeselectAll(bool bDestroyDeselected) {
    TArray<USceneComponent*> componentsToDeselect = SelectedComponents;
    for (auto& i : componentsToDeselect)
        DeselectComponent(i);
    //calling internal so as not to modify SelectedComponents until the last bit!
    SelectedComponents.Empty();
    UpdateGizmoPlacement();

    if (bDestroyDeselected) {
        for (auto& c : componentsToDeselect) {
            if (!IsValid(c)) continue; //a component that was in the same actor destroyed will be pending kill
            if (AActor* actor = c->GetOwner()) {
                //We destroy the actor if no components are left to destroy, or the system is currently ActorBased
                if (bComponentBased && actor->GetComponents().Num() > 1)
                    c->DestroyComponent(true);
                else
                    actor->Destroy();
            }
        }
    }

    return componentsToDeselect;
}

void UTwinLinkTransformerComponent::AddComponent_Internal(TArray<USceneComponent*>& OutComponentList
    , USceneComponent* Component) {
    //if (!Component) return; //assumes that previous have checked, since this is Internal.

    int32 Index = OutComponentList.Find(Component);

    if (INDEX_NONE == Index) //Component is not in list
    {
        OutComponentList.Emplace(Component);
        bool bImplementsInterface;
        Select(OutComponentList.Last(), &bImplementsInterface);
        OnComponentSelectionChange(Component, true, bImplementsInterface);
    }
    else if (bToggleSelectedInMultiSelection)
        DeselectComponentAtIndex_Internal(OutComponentList, Index);
}

void UTwinLinkTransformerComponent::DeselectComponent_Internal(TArray<USceneComponent*>& OutComponentList
    , USceneComponent* Component) {
    //if (!Component) return; //assumes that previous have checked, since this is Internal.

    int32 Index = OutComponentList.Find(Component);
    if (INDEX_NONE != Index)
        DeselectComponentAtIndex_Internal(OutComponentList, Index);
}

void UTwinLinkTransformerComponent::DeselectComponentAtIndex_Internal(
    TArray<USceneComponent*>& OutComponentList
    , int32 Index) {
    //if (!Component) return; //assumes that previous have checked, since this is Internal.
    if (OutComponentList.IsValidIndex(Index)) {
        USceneComponent* Component = OutComponentList[Index];
        bool bImplementsInterface;
        Deselect(Component, &bImplementsInterface);
        OutComponentList.RemoveAt(Index);
        OnComponentSelectionChange(Component, false, bImplementsInterface);
    }
}

void UTwinLinkTransformerComponent::SetGizmo() {
    //If there are selected components, then we see whether we need to create a new gizmo.
    if (SelectedComponents.Num() > 0) {
        bool bCreateGizmo = true;
        if (Gizmo.IsValid()) {
            if (CurrentTransformation == Gizmo->GetGizmoType()) {
                bCreateGizmo = false; // do not create gizmo if there is already a matching gizmo
            }
            else {
                // Destroy the current gizmo as the transformation types do not match
                Gizmo->Destroy();
                Gizmo.Reset();
            }
        }

        if (bCreateGizmo) {
            if (UWorld* world = GetWorld()) {
                UClass* GizmoClass = GetGizmoClass(CurrentTransformation);
                if (GizmoClass) {
                    Gizmo = Cast<ATwinLinkBaseGizmo>(world->SpawnActor(GizmoClass));
                    Gizmo->OnGizmoStateChange.AddDynamic(this, &UTwinLinkTransformerComponent::OnGizmoStateChanged);
                }
            }
        }
    }
    //Since there are no selected components, we must destroy any gizmos present
    else {
        if (Gizmo.IsValid()) {
            Gizmo->Destroy();
            Gizmo.Reset();
        }
    }
}

void UTwinLinkTransformerComponent::UpdateGizmoPlacement() {
    SetGizmo();
    //means that there are no active gizmos (no selections) so nothing to do in this func
    if (!Gizmo.IsValid()) return;

    USceneComponent* ComponentToAttachTo = nullptr;

    switch (GizmoPlacement) {
    case ETwinLinkGizmoPlacement::GP_OnFirstSelection:
        ComponentToAttachTo = SelectedComponents[0]; break;
    case ETwinLinkGizmoPlacement::GP_OnLastSelection:
        ComponentToAttachTo = SelectedComponents.Last(); break;
    }

    if (ComponentToAttachTo) {
        Gizmo->AttachToComponent(ComponentToAttachTo
            , FAttachmentTransformRules::SnapToTargetIncludingScale);
    }
    else {
        //  Gizmo->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

    Gizmo->UpdateGizmoSpace(CurrentSpaceType);
}

///////////////////////// NETWORKING ////////////////////////////////////////////////////////////////////////

void UTwinLinkTransformerComponent::ReplicatedMouseTraceByObjectTypes(float TraceDistance
    , TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels, bool bAppendToList) {
    FVector start, end;
    if (GetMouseStartEndPoints(TraceDistance, start, end)) {
        bool bTraceSuccessful = TraceByObjectTypes(start, end
            , CollisionChannels
            , TArray<AActor*>(), bAppendToList);

        //Server
        if (GetOwner()->GetLocalRole() == ROLE_Authority)
            ReplicateServerTraceResults(bTraceSuccessful, bAppendToList);
        //Client
        else {
            if (!bTraceSuccessful && !bAppendToList)
                ServerDeselectAll(false);
            else {
                // If a Local Trace was on a Gizmo, just tell the Server that we
                // have hit our Gizmo and just change the Domain there.
                // Else, do the Server Trace
                if (CurrentDomain == ETwinLinkTransformationDomain::TD_None)
                    ServerTraceByObjectTypes(start, end, CollisionChannels, bAppendToList);
                else
                    ServerSetDomain(CurrentDomain);
            }
        }
    }
}

void UTwinLinkTransformerComponent::ReplicatedMouseTraceByChannel(float TraceDistance
    , TEnumAsByte<ECollisionChannel> CollisionChannel, bool bAppendToList) {
    FVector start, end;
    if (GetMouseStartEndPoints(TraceDistance, start, end)) {
        bool bTraceSuccessful = TraceByChannel(start, end
            , CollisionChannel
            , TArray<AActor*>(), bAppendToList);

        //Server
        if (GetOwner()->GetLocalRole() == ROLE_Authority)
            ReplicateServerTraceResults(bTraceSuccessful, bAppendToList);
        //Client
        else {
            if (!bTraceSuccessful && !bAppendToList)
                ServerDeselectAll(false);

            // If a Local Trace was on a Gizmo, just tell the Server that we
            // have hit our Gizmo and just change the Domain there.
            // Else, do the Server Trace
            if (CurrentDomain == ETwinLinkTransformationDomain::TD_None)
                ServerTraceByChannel(start, end, CollisionChannel, bAppendToList);
            else
                ServerSetDomain(CurrentDomain);
        }
    }
}

void UTwinLinkTransformerComponent::ReplicatedMouseTraceByProfile(float TraceDistance
    , const FName& ProfileName, bool bAppendToList) {
    FVector start, end;
    if (GetMouseStartEndPoints(TraceDistance, start, end)) {
        bool bTraceSuccessful = TraceByProfile(start, end
            , ProfileName
            , TArray<AActor*>(), bAppendToList);

        //Server
        if (GetOwner()->GetLocalRole() == ROLE_Authority)
            ReplicateServerTraceResults(bTraceSuccessful, bAppendToList);
        //Client
        else {
            if (!bTraceSuccessful && !bAppendToList)
                ServerDeselectAll(false);

            // If a Local Trace was on a Gizmo, just tell the Server that we
            // have hit our Gizmo and just change the Domain there.
            // Else, do the Server Trace
            if (CurrentDomain == ETwinLinkTransformationDomain::TD_None)
                ServerTraceByProfile(start, end, ProfileName, bAppendToList);
            else
                ServerSetDomain(CurrentDomain);
        }
    }
}

TArray<AActor*> UTwinLinkTransformerComponent::GetIgnoredActorsForServerTrace() const {
    TArray<AActor*> ignoredActors;
    //Ignore Gizmo in Server Trace Test if it's not Server controlling Pawn (since Gizmo is relative)
    const auto Controller = UGameplayStatics::GetPlayerController(GetOwner(), 0);

    const auto IsLocallyControlled = Controller && Controller->IsLocalController();

    if (!IsLocallyControlled) {
        if (Gizmo.IsValid())
            ignoredActors.Add(Gizmo.Get());
    }
    return ignoredActors;
}

void UTwinLinkTransformerComponent::ReplicateServerTraceResults(bool bTraceSuccessful, bool bAppendToList) {
    //Only perform this on Clients
    if (!GetOwner()->HasAuthority()) {
        if (!bTraceSuccessful && !bAppendToList)
            DeselectAll(false);
        MulticastSetDomain(CurrentDomain);
        MulticastSetSelectedComponents(SelectedComponents);
    }
}

void UTwinLinkTransformerComponent::LogSelectedComponents() {
    UE_LOG(LogTwinLinkGizmo, Log, TEXT("******************** SELECTED COMPONENTS LOG START ********************"));
    UE_LOG(LogTwinLinkGizmo, Log, TEXT("   * Selected Component Count: %d"), SelectedComponents.Num());
    UE_LOG(LogTwinLinkGizmo, Log, TEXT("   * -------------------------------- "));
    for (int32 i = 0; i < SelectedComponents.Num(); ++i) {
        USceneComponent* cmp = SelectedComponents[i];
        FString message = "Component: ";
        if (cmp) {
            message += cmp->GetName() + "\tOwner: ";
            if (AActor* owner = cmp->GetOwner())
                message += owner->GetName();
            else
                message += TEXT("[INVALID]");
        }
        else
            message += TEXT("[INVALID]");

        UE_LOG(LogTwinLinkGizmo, Log, TEXT("   * [%d] %s"), i, *message);
    }

    UE_LOG(LogTwinLinkGizmo, Log, TEXT("******************** SELECTED COMPONENTS LOG END   ********************"));
}

bool UTwinLinkTransformerComponent::ServerTraceByObjectTypes_Validate(
    const FVector& StartLocation, const FVector& EndLocation
    , const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
    , bool bAppendToList) {
    return true;
}

void UTwinLinkTransformerComponent::ServerTraceByObjectTypes_Implementation(
    const FVector& StartLocation, const FVector& EndLocation
    , const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels
    , bool bAppendToList) {
    bool bTraceSuccessful = TraceByObjectTypes(StartLocation, EndLocation, CollisionChannels
        , GetIgnoredActorsForServerTrace(), bAppendToList);

    if (!bTraceSuccessful && !bAppendToList)
        //check whether trace was successful and we're not doing multi selection
        DeselectAll(false);

    MulticastSetDomain(CurrentDomain);
    MulticastSetSelectedComponents(SelectedComponents);
}

bool UTwinLinkTransformerComponent::ServerTraceByChannel_Validate(
    const FVector& StartLocation, const FVector& EndLocation
    , ECollisionChannel TraceChannel, bool bAppendToList) {
    return true;
}

void UTwinLinkTransformerComponent::ServerTraceByChannel_Implementation(
    const FVector& StartLocation, const FVector& EndLocation
    , ECollisionChannel TraceChannel, bool bAppendToList) {
    bool bTraceSuccessful = TraceByChannel(StartLocation, EndLocation, TraceChannel
        , GetIgnoredActorsForServerTrace(), bAppendToList);

    if (!bTraceSuccessful && !bAppendToList)
        //check whether trace was successful and we're not doing multi selection
        DeselectAll(false);

    MulticastSetDomain(CurrentDomain);
    MulticastSetSelectedComponents(SelectedComponents);
}

bool UTwinLinkTransformerComponent::ServerTraceByProfile_Validate(const FVector& StartLocation
    , const FVector& EndLocation, const FName& ProfileName, bool bAppendToList) {
    return true;
}

void UTwinLinkTransformerComponent::ServerTraceByProfile_Implementation(
    const FVector& StartLocation, const FVector& EndLocation
    , const FName& ProfileName, bool bAppendToList) {
    bool bTraceSuccessful = TraceByProfile(StartLocation, EndLocation, ProfileName
        , GetIgnoredActorsForServerTrace(), bAppendToList);

    if (!bTraceSuccessful && !bAppendToList)
        //check whether trace was successful and we're not doing multi selection
        DeselectAll(false);

    MulticastSetDomain(CurrentDomain);
    MulticastSetSelectedComponents(SelectedComponents);
}

bool UTwinLinkTransformerComponent::ServerClearDomain_Validate() {
    return true;
}
void UTwinLinkTransformerComponent::ServerClearDomain_Implementation() {
    MulticastClearDomain();
}

void UTwinLinkTransformerComponent::MulticastClearDomain_Implementation() {
    ClearDomain();
}

bool UTwinLinkTransformerComponent::ServerApplyTransform_Validate(const FTransform& DeltaTransform) {
    return true;
}
void UTwinLinkTransformerComponent::ServerApplyTransform_Implementation(const FTransform& DeltaTransform) {
    MulticastApplyTransform(DeltaTransform);
}

void UTwinLinkTransformerComponent::MulticastApplyTransform_Implementation(const FTransform& DeltaTransform) {
    const auto Controller = UGameplayStatics::GetPlayerController(GetOwner(), 0);

    if (Controller && !Controller->IsLocalController()) //only apply to others
        ApplyDeltaTransform(DeltaTransform);
}

void UTwinLinkTransformerComponent::ReplicateFinishTransform() {
    ServerClearDomain();
    ServerApplyTransform(NetworkDeltaTransform);
    ResetDeltaTransform(NetworkDeltaTransform);
}

bool UTwinLinkTransformerComponent::ServerDeselectAll_Validate(bool bDestroySelected) {
    return true;
}
void UTwinLinkTransformerComponent::ServerDeselectAll_Implementation(bool bDestroySelected) {
    MulticastDeselectAll(bDestroySelected);
}

void UTwinLinkTransformerComponent::MulticastDeselectAll_Implementation(bool bDestroySelected) {
    DeselectAll(bDestroySelected);
}

bool UTwinLinkTransformerComponent::ServerSetSpaceType_Validate(ETwinLinkSpaceType Space) {
    return true;
}
void UTwinLinkTransformerComponent::ServerSetSpaceType_Implementation(ETwinLinkSpaceType Space) {
    MulticastSetSpaceType(Space);
}

void UTwinLinkTransformerComponent::MulticastSetSpaceType_Implementation(ETwinLinkSpaceType Space) {
    SetSpaceType(Space);
}

bool UTwinLinkTransformerComponent::ServerSetTransformationType_Validate(ETwinLinkTransformationType Transformation) {
    return true;
}
void UTwinLinkTransformerComponent::ServerSetTransformationType_Implementation(ETwinLinkTransformationType Transformation) {
    MulticastSetTransformationType(Transformation);
}

void UTwinLinkTransformerComponent::MulticastSetTransformationType_Implementation(ETwinLinkTransformationType Transformation) {
    SetTransformationType(Transformation);
}

bool UTwinLinkTransformerComponent::ServerSetComponentBased_Validate(bool bIsComponentBased) {
    return true;
}
void UTwinLinkTransformerComponent::ServerSetComponentBased_Implementation(bool bIsComponentBased) {
    MulticastSetComponentBased(bIsComponentBased);
}

void UTwinLinkTransformerComponent::MulticastSetComponentBased_Implementation(bool bIsComponentBased) {
    SetComponentBased(bIsComponentBased);
}

bool UTwinLinkTransformerComponent::ServerSetRotateOnLocalAxis_Validate(bool bRotateLocalAxis) {
    return true;
}
void UTwinLinkTransformerComponent::ServerSetRotateOnLocalAxis_Implementation(bool bRotateLocalAxis) {
    MulticastSetRotateOnLocalAxis(bRotateLocalAxis);
}

void UTwinLinkTransformerComponent::MulticastSetRotateOnLocalAxis_Implementation(bool bRotateLocalAxis) {
    SetRotateOnLocalAxis(bRotateLocalAxis);
}

#include "TimerManager.h"
bool UTwinLinkTransformerComponent::ServerCloneSelected_Validate(bool bSelectNewClones, bool bAppendToList) {
    return true;
}
void UTwinLinkTransformerComponent::ServerCloneSelected_Implementation(bool bSelectNewClones
    , bool bAppendToList) {
    if (bComponentBased) {
        UE_LOG(LogTwinLinkGizmo, Warning, TEXT("** Component Cloning is currently not supported in a Network Environment :( **"));
        // see PluginLimitations.txt for a reason why Component Cloning is not supported
        return;
    }

    auto ComponentListCopy = GetSelectedComponents();

    //just create 'em, not select 'em (we select 'em later)
    auto CloneList = CloneFromList(ComponentListCopy);

    //if we have to select the new clones, multicast for these new objects
    if (bSelectNewClones) {
        SelectMultipleComponents(CloneList, bAppendToList);
        UnreplicatedComponentClones = CloneList;

        //Timer to loop until all Unreplicated Actors have finished replicating!
        if (UWorld* world = GetWorld()) {
            if (!CheckUnrepTimerHandle.IsValid())
                world->GetTimerManager().SetTimer(CheckUnrepTimerHandle, this
                    , &UTwinLinkTransformerComponent::CheckUnreplicatedActors
                    , CloneReplicationCheckFrequency, true, 0.0f);
        }
    }
}

void UTwinLinkTransformerComponent::CheckUnreplicatedActors() {
    int32 RemoveCount = 0;
    float timeElapsed = GetOwner()->GetWorldTimerManager().GetTimerElapsed(CheckUnrepTimerHandle);
    for (auto& c : UnreplicatedComponentClones) {
        // rather than calling "IsSupportedForNetworking" (which returns true all the time)
        // call HasActorBegunPlay (which means we are sure the BeginPlay for AActor has finished completely.
        // and so we can safely send this reference over the network.
        if (c && c->HasBegunPlay() && c->IsSupportedForNetworking() &&
            timeElapsed > MinimumCloneReplicationTime) // Make sure there's a minimum time...
            ++RemoveCount;
    }

    // Remove with performance (Swapping). We don't care about the order in this case
    // since the role of this is to check that all unreplicated components have been replicated.
    UnreplicatedComponentClones.RemoveAtSwap(0, RemoveCount);

    // check if all clones have been replicated
    if (UnreplicatedComponentClones.Num() == 0) {
        //stop calling this if no more unreplicated actors
        GetOwner()->GetWorldTimerManager().ClearTimer(CheckUnrepTimerHandle);

        UE_LOG(LogTwinLinkGizmo, Log, TEXT("[SERVER] Time Elapsed for %d Replicated Actors to replicate: %f")
            , SelectedComponents.Num(), timeElapsed);

        //send all the selected replicated actors!
        MulticastSetSelectedComponents(SelectedComponents);
    }
}

bool UTwinLinkTransformerComponent::ServerSetDomain_Validate(ETwinLinkTransformationDomain Domain) {
    return true;
}
void UTwinLinkTransformerComponent::ServerSetDomain_Implementation(ETwinLinkTransformationDomain Domain) {
    MulticastSetDomain(Domain);
}

void UTwinLinkTransformerComponent::MulticastSetDomain_Implementation(ETwinLinkTransformationDomain Domain) {
    SetDomain(Domain);
}

bool UTwinLinkTransformerComponent::ServerSyncSelectedComponents_Validate() {
    return true;
}
void UTwinLinkTransformerComponent::ServerSyncSelectedComponents_Implementation() {
    MulticastSetSelectedComponents(SelectedComponents);
}

void UTwinLinkTransformerComponent::MulticastSetSelectedComponents_Implementation(
    const TArray<USceneComponent*>& Components) {
    if (GetOwner()->GetLocalRole() < ROLE_Authority) {
        UE_LOG(LogTwinLinkGizmo, Log, TEXT("MulticastSelect ComponentCount: %d"), Components.Num());
    }

    DeselectAll(); //calling here because Selecting MultipleComponents empty is not going to call Deselect all
    SelectMultipleComponents(Components, true);

    //Tells whether we have Selected the exact number of components that came in
    // or there was a nullptr in Components and therefore there is a difference.
    // if there is a difference we will need to resync
    bResyncSelection = (Components.Num() != SelectedComponents.Num());
    if (bResyncSelection) {
        //Timer to loop until all Unreplicated Actors have finished replicating!
        if (UWorld* world = GetWorld()) {
            if (!ResyncSelectionTimerHandle.IsValid())
                world->GetTimerManager().SetTimer(ResyncSelectionTimerHandle, this
                    , &UTwinLinkTransformerComponent::ResyncSelection
                    , 0.1f, true, 0.0f);
        }
    }

    if (GetOwner()->GetLocalRole() < ROLE_Authority) {
        UE_LOG(LogTwinLinkGizmo, Log, TEXT("Selected ComponentCount: %d"), SelectedComponents.Num());
    }
}

void UTwinLinkTransformerComponent::ResyncSelection() {
    //TODO
    if (bResyncSelection) {
        UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Resyncing Selection"));
        ServerSyncSelectedComponents();
    }
    else {
        UE_LOG(LogTwinLinkGizmo, Warning, TEXT("Resyncing FINISHED"));
        //stop calling this if no more resync is needed
        GetOwner()->GetWorldTimerManager().ClearTimer(ResyncSelectionTimerHandle);
    }
}