// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityEditDialogBase.h"
#include "TwinLinkCommon.h"

#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkFacilityInfo.h"
#include "NavSystem/TwinLinkNavSystem.h"
#include "NavSystem/TwinLinkNavSystemEntranceLocator.h"

void UTwinLinkFacilityEditDialogBase::Setup(UTwinLinkFacilityInfo* Info) {
    check(Info);

    if (FacilityInfo.IsValid()) {
        check(EvOnChangedHnd.IsValid());
        FacilityInfo->EvOnChanged.Remove(EvOnChangedHnd);
        EvOnChangedHnd.Reset();
    }

    FacilityInfo = Info;

    // 変更された際に更新関数を呼び出す
    EvOnChangedHnd = FacilityInfo->EvOnChanged.AddLambda([this]() {
        const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
        check(FacilityInfoSys.IsValid());
        OnChangedInfo();
        });

    if (!EntranceLocatorNode)
        EntranceLocatorNode = new FTwinLinkEntranceLocatorWidgetNode(this);

    if (EntranceLocatorNode)
        EntranceLocatorNode->SetEntranceLocation(Info);

    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    OnChangedCategoryGroup(FacilityInfoSys->GetCategoryDisplayNameCollection());
    OnChangedInfo();
}

FString UTwinLinkFacilityEditDialogBase::GetDisplayCategoryName() const {
    check(FacilityInfo.IsValid());

    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    return FacilityInfoSys->GetCategoryDisplayName(FacilityInfo->GetCategory());
}

void UTwinLinkFacilityEditDialogBase::BeginDestroy() {
    Super::BeginDestroy();
    if (EntranceLocatorNode)
        delete EntranceLocatorNode;
}

void UTwinLinkFacilityEditDialogBase::RequestEdit(const FString& Name, const FString& Category, const FString& ImageFileName, const FString& Guide, const FString& SpotInfo) {
    auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());

    std::optional<FVector> Entrance;
    if (EntranceLocatorNode)
        Entrance = EntranceLocatorNode->GetEntranceLocation();

    const auto IsSuc = FacilityInfoSys->EditFacilityInfo(
        FacilityInfo,
        Name,
        Category,
        ImageFileName,
        Guide,
        SpotInfo,
        Entrance
    );

    if (IsSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed RequestEdit()"));
        OnFailedRequestEdit();
        return;
    }

    FacilityInfoSys->ExportFacilityInfo();

    OnSuccessRequestEdit();
}