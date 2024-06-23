// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkCommon.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"

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

float TwinLinkGraphicsEnv::GetEmissiveBoostFromEnv(UWorld* World) {
    float EmissiveBoost = 1.0;
    //エミッシブ強度調整
    bool bIsNight = GetNightIntensity(World) >= 1.0f * (3.0f / 4.0f);    // 3/4を夜の基準値とする
    EmissiveBoost = bIsNight ? 1.0 : 10000.0;
    return EmissiveBoost;
}

float TwinLinkGraphicsEnv::GetNightIntensity(UWorld* World) {
    TArray<TObjectPtr<AActor>> Actors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
    for (const auto& Actor : Actors) {
        const auto SkyLight = Actor->GetComponentByClass(USkyLightComponent::StaticClass());
        if (SkyLight != nullptr) {
            const auto Component = Cast<USceneComponent>(Actor->GetComponentByClass(UDirectionalLightComponent::StaticClass()));
            if (Component == nullptr)
                continue;
            //ライトのYが+の場合は夜と判定
            return Component->GetRelativeRotation().Pitch > 0 ? 1.0 : 0.0;
        }
    }
    return 0.0;
}
