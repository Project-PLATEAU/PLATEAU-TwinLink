// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkSceneComponentBase.h"

bool UTwinLinkSceneComponentBase::bIsEnableLineTrace = true;

// Sets default values for this component's properties
UTwinLinkSceneComponentBase::UTwinLinkSceneComponentBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTwinLinkSceneComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UTwinLinkSceneComponentBase::BeginDestroy() {
    // ...

    Super::BeginDestroy();
}


// Called every frame
void UTwinLinkSceneComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTwinLinkSceneComponentBase::EnableLineTrace(bool InIsEnableLineTrace) {
    bIsEnableLineTrace = InIsEnableLineTrace;
}

bool UTwinLinkSceneComponentBase::IsEnableLineTrace() const {
    return bIsEnableLineTrace;
}

