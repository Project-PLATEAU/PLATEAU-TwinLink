// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAddFacilityDialogBase.h"

#include "Misc/TwinLinkActorEx.h"
#include "Kismet/GameplayStatics.h"
#include "TwinLinkCommon.h"

#include "TwinLinkPersistentPaths.h"
#include "TwinLinkWorldViewer.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "Misc/TwinLinkWidgetEx.h"
#include "NavSystem/TwinLinkNavSystem.h"
#include "NavSystem/TwinLinkNavSystemEntranceLocator.h"
#include "NavSystem/TwinLinkNavSystemPathFinder.h"

void UTwinLinkAddFacilityDialogBase::Setup() {
    const auto WorldViewer =
        ATwinLinkWorldViewer::GetInstance(GetWorld());

    if (WorldViewer.IsValid() == false)
        return;
    check(WorldViewer.IsValid());
    // クリック時にイベントを追加する
    WorldViewer->EvOnClickedFacility.AddLambda([this](FHitResult HitResult) {
        // #NOTE : 追加ダイアログが出ているときはクリックしても選択建物変更しない
        if (TwinLinkWidgetEx::IsVisibleIncludeOuter(this))
            return;

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

void UTwinLinkAddFacilityDialogBase::OnShowDialog() {
    // ダイアログ開く瞬間は自分が見えてないので強制で設定する
    if (!EntranceLocatorNode)
        EntranceLocatorNode = new FTwinLinkEntranceLocatorWidgetNode(this);
    if (EntranceLocatorNode)
        EntranceLocatorNode->SetDefaultEntranceLocation(FeatureID, true);
}

void UTwinLinkAddFacilityDialogBase::BeginDestroy() {
    Super::BeginDestroy();
    if (EntranceLocatorNode)
        delete EntranceLocatorNode;
}

const UObject* UTwinLinkAddFacilityDialogBase::GetSelectedObjectIfVisible() {
    if (TwinLinkWidgetEx::IsVisibleIncludeOuter(this) == false)
        return nullptr;

    return GetTargetFeatureComponent();
}

void UTwinLinkAddFacilityDialogBase::AddFacilityInfo(const FString& InName, const FString& InCategory, const FString& InImageFileName, const FString& InDescription, const FString& InSpotInfo) {
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();

    std::optional<FVector> Entrance;
    if (EntranceLocatorNode)
        Entrance = EntranceLocatorNode->GetEntranceLocation();

    bool bIsValid = UTwinLinkFacilityInfo::IsValid(
        InName,
        InCategory,
        FeatureID,
        InImageFileName,
        InDescription,
        InSpotInfo,
        Entrance.has_value() ? TArray<FVector>{ Entrance.value() } : TArray<FVector>());

    if (IsValid == false) {
        OnFailedFacilityInfo();
        return;
    }

    FacilityInfoSys->AddFacilityInfo(InName, InCategory, FeatureID, InImageFileName, InDescription, InSpotInfo, Entrance);
    FacilityInfoSys->ExportFacilityInfo();
    OnAddedFacilityInfo();
}

FString UTwinLinkAddFacilityDialogBase::CreateImageFilePath(const FString& InFileName) {
    return TwinLinkPersistentPaths::CreateFacilityImagePath(InFileName);
}
