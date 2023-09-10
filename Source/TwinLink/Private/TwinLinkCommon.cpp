// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkCommon.h"
#include "Kismet/GameplayStatics.h"

#include "TwinLinkWorldViewer.h"

DEFINE_LOG_CATEGORY(TwinLinkLogSerialization);
DEFINE_LOG_CATEGORY(TwinLinkLogMemory);
DEFINE_LOG_CATEGORY(TwinLinkLogInit);
DEFINE_LOG_CATEGORY(TwinLinkLogExit);
DEFINE_LOG_CATEGORY(TwinLinkLogExec);
DEFINE_LOG_CATEGORY(TwinLinkLogProcess);
DEFINE_LOG_CATEGORY(TwinLinkLogLoad);
DEFINE_LOG_CATEGORY(TwinLinkLogVirtualization);
DEFINE_LOG_CATEGORY(TwinLinkLogTemp);

TWeakObjectPtr<ACharacter> TwinLinkWorldViewerHelper::GetInstance(UWorld* World) {
    AActor* ViewPointActor = UGameplayStatics::GetActorOfClass(World, ATwinLinkWorldViewer::StaticClass());
    if (ViewPointActor == nullptr) {
        return nullptr;
    }

    ATwinLinkWorldViewer* ApplicableTargets = Cast<ATwinLinkWorldViewer>(ViewPointActor);
    if (ApplicableTargets == nullptr) {
        return nullptr;
    }

    return ApplicableTargets;
}
