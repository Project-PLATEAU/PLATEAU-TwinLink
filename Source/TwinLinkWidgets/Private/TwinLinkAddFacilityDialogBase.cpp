// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAddFacilityDialogBase.h"
#include "Kismet/GameplayStatics.h"
#include "TwinLinkCommon.h"

#include "TwinLinkPersistentPaths.h"
#include "TwinLinkWorldViewer.h"
#include "TwinLinkFacilityInfoSystem.h"

void UTwinLinkAddFacilityDialogBase::Setup() {
    const auto WorldViewer = 
        ATwinLinkWorldViewer::GetInstance(GetWorld());

    if (WorldViewer.IsValid() == false)
        return;
    check(WorldViewer.IsValid());

    // クリック時にイベントを追加する
    WorldViewer->EvOnClickedFacility.AddLambda([this](FHitResult HitResult) {
        FeatureID = HitResult.Component->GetName();
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("FeatureID %s"), *FeatureID);

        // 施設がクリックされた時に呼び出される
        OnSelectFeatureID(FeatureID);
        });

    // 施設カテゴリ群を設定する
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    SyncCategoryCollectionWidget(
        FacilityInfoSys.Get()->GetCategoryDisplayNameCollection());

}

void UTwinLinkAddFacilityDialogBase::AddFacilityInfo(const FString& InName, const FString& InCategory, const FString& InImageFileName, const FString& InDescription, const FString& InSpotInfo) {
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();

    if (FacilityInfoSys->CheckAddableFacilityInfo(InName, FeatureID) == false) {
        OnFailedFacilityInfo();
        return;
    }

    FacilityInfoSys->AddFacilityInfo(InName, InCategory, FeatureID, InImageFileName, InDescription, InSpotInfo);
    FacilityInfoSys->ExportFacilityInfo();

    OnAddedFacilityInfo();

}

FString UTwinLinkAddFacilityDialogBase::CreateImageFilePath(const FString& InFileName) {
    return TwinLinkPersistentPaths::CreateFacilityImagePath(InFileName);
}
