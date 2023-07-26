// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkTickSystem.h"
#include "TwinLinkCommon.h"

void UTwinLinkTickSystem::Initialize(FSubsystemCollectionBase& Collection) {
    const auto ReserveSize = 10;    // てきとうな値
    TickableCollection.Reserve(ReserveSize);

    bTickable = true;
    bCanAddSubSystem = true;

}

void UTwinLinkTickSystem::Deinitialize() {
}

void UTwinLinkTickSystem::AddSubSystem(ITwinLinkTickable* SubSystem) {
    check(bCanAddSubSystem);
    TickableCollection.Add(SubSystem);
}

TStatId UTwinLinkTickSystem::GetStatId() const {
    RETURN_QUICK_DECLARE_CYCLE_STAT(UTwinLinkTickSystem, STATGROUP_Tickables);
}

void UTwinLinkTickSystem::Tick(float DeltaTime) {
    bCanAddSubSystem = false;
    bTickable = false;

    for (auto Tickable : TickableCollection) {
        Tickable->Tick(DeltaTime);
    }

    if (EvDelayExec.IsBound()) {
        EvDelayExec.Broadcast();
        EvDelayExec.Clear();
    }

    bCanAddSubSystem = true;
    bTickable = true;
}
