// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityAttributeBase.h"
#include "TwinLinkCommon.h"
#include "TwinLinkWorldViewer.h"

// Sets default values for this component's properties
UTwinLinkFacilityAttributeBase::UTwinLinkFacilityAttributeBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UTwinLinkFacilityAttributeBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
    check(EvOnClickedHnd.IsValid() == false);
    const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
    EvOnClickedHnd = WorldViewer->EvOnClicked.AddLambda([this]() {
        LineTraceToCityObject();
        });
}

void UTwinLinkFacilityAttributeBase::BeginDestroy() {
    // ...

    if (EvOnClickedHnd.IsValid()) {
        const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
        if (WorldViewer.IsValid()) {
            WorldViewer->EvOnClicked.Remove(EvOnClickedHnd);
        }
    }

    Super::BeginDestroy();
}


// Called every frame
void UTwinLinkFacilityAttributeBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

