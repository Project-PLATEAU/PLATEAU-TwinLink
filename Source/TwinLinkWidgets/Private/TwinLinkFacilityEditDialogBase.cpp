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
    if (const auto Entrance = ATwinLinkNavSystem::GetEntranceLocator(GetWorld()))
        Entrance->SetEntranceLocation(Info);

    // 変更された際に更新関数を呼び出す
    EvOnChangedHnd = FacilityInfo->EvOnChanged.AddLambda([this]() {
        const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
        check(FacilityInfoSys.IsValid());
        OnChangedInfo();
        });

    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    OnChangedCategoryGroup(FacilityInfoSys->GetCategoryDisplayNameCollection());
    OnChangedInfo();

    OnVisibilityChanged.AddDynamic(this, &UTwinLinkFacilityEditDialogBase::OnChangeActive);
}

FString UTwinLinkFacilityEditDialogBase::GetDisplayCategoryName() const {
    check(FacilityInfo.IsValid());

    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    return FacilityInfoSys->GetCategoryDisplayName(FacilityInfo->GetCategory());
}

void UTwinLinkFacilityEditDialogBase::RequestEdit(const FString& Name, const FString& Category, const FString& ImageFileName, const FString& Guide, const FString& SpotInfo) {
    auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());

    std::optional<FVector> Entrance;
    if (const auto EntranceLocator = ATwinLinkNavSystem::GetEntranceLocator(GetWorld()))
            Entrance = EntranceLocator->GetPathLocation();

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

void UTwinLinkFacilityEditDialogBase::OnChangeActive(ESlateVisibility Visible)
{
    if (auto NavSys = ATwinLinkNavSystem::GetInstance(GetWorld()))
        NavSys->SetEntranceLocatorActive(Visible == ESlateVisibility::Visible);
}
