// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAddFacilityDialogBase.h"

#include "TwinLinkActorEx.h"
#include "Kismet/GameplayStatics.h"
#include "TwinLinkCommon.h"

#include "TwinLinkWorldViewer.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "NavSystem/TwinLinkNavSystem.h"
#include "NavSystem/TwinLinkNavSystemEntranceLocator.h"
#include "NavSystem/TwinLinkNavSystemPathFinder.h"

void UTwinLinkAddFacilityDialogBase::Setup() {
    const auto WorldViewer =
        ATwinLinkWorldViewer::GetInstance(GetWorld());

    check(WorldViewer.IsValid());
    // クリック時にイベントを追加する
    WorldViewer->EvOnClickedFacility.AddLambda([this](FHitResult HitResult) {
        FeatureID = HitResult.Component->GetName();
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("FeatureID %s"), *FeatureID);

        // 施設がクリックされた時に呼び出される
        OnSelectFeatureID(FeatureID);

        if (EntranceLocatorNode)
            EntranceLocatorNode->SetDefaultEntranceLocation(FeatureID);
        });

    // 施設カテゴリ群を設定する
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    SyncCategoryCollectionWidget(
        FacilityInfoSys.Get()->GetCategoryDisplayNameCollection());

    if (!EntranceLocatorNode)
        EntranceLocatorNode = new FTwinLinkEntranceLocatorWidgetNode(this);
}

const UPrimitiveComponent* UTwinLinkAddFacilityDialogBase::GetTargetFeatureComponent() const {
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    return FacilityInfoSys->FindFacility(FeatureID).Get();
}

void UTwinLinkAddFacilityDialogBase::BeginDestroy() {
    Super::BeginDestroy();
    if (EntranceLocatorNode)
        delete EntranceLocatorNode;
}

void UTwinLinkAddFacilityDialogBase::AddFacilityInfo(const FString& InName, const FString& InCategory, const FString& InImageFileName, const FString& InDescription, const FString& InSpotInfo) {
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();

    if (FacilityInfoSys->CheckAddableFacilityInfo(InName, FeatureID) == false) {
        OnFailedFacilityInfo();
        return;
    }

    std::optional<FVector> Entrance;
    if (EntranceLocatorNode)
        Entrance = EntranceLocatorNode->GetEntranceLocation();

    FacilityInfoSys->AddFacilityInfo(InName, InCategory, FeatureID, InImageFileName, InDescription, InSpotInfo, Entrance);
    FacilityInfoSys->ExportFacilityInfo();
    OnAddedFacilityInfo();
}
