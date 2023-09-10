// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityItem.h"

#include "TwinLinkCommon.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkFacilityInfo.h"


void UTwinLinkFacilityItem::OnSetup(UObject* Data) const {
    check(Data);

    auto CastedElement = Cast<UTwinLinkFacilityInfo>(Data);
    check(CastedElement);

    //// 登録済みのイベントを削除
    //if (EvOnChangedHnd.IsValid()) {
    //    CastedElement->EvOnChanged.Remove(EvOnChangedHnd);
    //    EvOnChangedHnd.Reset();
    //}

    //EvOnChangedHnd = CastedElement->EvOnChanged.AddLambda([]() {
    //    Onchanged
    //    });
}

void UTwinLinkFacilityItem::OnRequestRemove(UObject* Data) const {
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());

    auto CastedElement = Cast<UTwinLinkFacilityInfo>(Data);
    check(CastedElement);

    FacilityInfoSys.Get()->RemoveFacilityInfo(CastedElement);
    FacilityInfoSys.Get()->ExportFacilityInfo();

}

void UTwinLinkFacilityItem::OnRequestEdit(UObject* Data, const FString& NewName) const {

}

FText UTwinLinkFacilityItem::OnGetName(UObject* Data) const {
    auto CastedData = Cast<UTwinLinkFacilityInfo>(Data);
    check(CastedData);
    return FText::FromString(CastedData->GetName());
}

void UTwinLinkFacilityItem::OnRequestSelect(UObject* Data) const {
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());

    auto CastedElement = Cast<UTwinLinkFacilityInfo>(Data);
    check(CastedElement);


    const auto Facility = FacilityInfoSys->FindFacility(CastedElement);
    if (Facility.IsValid()) {
        FacilityInfoSys->MoveCharactersNearTheFacility(Facility);
    }
    else {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed find facility"));
    }

}
