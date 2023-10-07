// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkBuildingFloorInfoItem.h"

#include "TwinLinkCommon.h"
#include "TwinLinkFloorInfoSystem.h"
#include "TwinLinkFloorInfo.h"

void UTwinLinkBuildingFloorInfoItem::OnSetup(UObject* Data) const {
}

void UTwinLinkBuildingFloorInfoItem::OnRequestRemove(UObject* Data) const {
    const auto FloorInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(FloorInfoSys.IsValid());

    auto CastedElement = Cast<UTwinLinkFloorInfo>(Data);
    check(CastedElement);

    const auto Key = FloorInfoSys->GetKeyBySelectedFloor();
    check(Key.IsEmpty() == false);

    FloorInfoSys.Get()->RemoveFloorInfo(Key, CastedElement);
    FloorInfoSys.Get()->ExportFloorInfo();
}

void UTwinLinkBuildingFloorInfoItem::OnRequestEdit(UObject* Data, const FString& NewName) const {
}

FText UTwinLinkBuildingFloorInfoItem::OnGetName(UObject* Data) const {
    auto CastedData = Cast<UTwinLinkFloorInfo>(Data);
    check(CastedData);
    return FText::FromString(CastedData->GetName());
}

void UTwinLinkBuildingFloorInfoItem::OnRequestSelect(UObject* Data) const {
}
