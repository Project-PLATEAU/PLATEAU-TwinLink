// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkRoomGetterBase.h"

UTwinLinkRoomGetterBase::UTwinLinkRoomGetterBase() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;

    // ...
}

void UTwinLinkRoomGetterBase::BeginPlay() {
    Super::BeginPlay();

    // ...
    check(EvOnClickedHnd.IsValid() == false);
    const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
    if (WorldViewer.IsValid()) {
        EvOnClickedHnd = WorldViewer->EvOnClicked.AddLambda([this]() {
            LineTraceToRoom();
            });
    }
}

void UTwinLinkRoomGetterBase::BeginDestroy() {
    // ...

    if (EvOnClickedHnd.IsValid()) {
        const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
        if (WorldViewer.IsValid()) {
            WorldViewer->EvOnClicked.Remove(EvOnClickedHnd);
        }
    }

    Super::BeginDestroy();
}

void UTwinLinkRoomGetterBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}
