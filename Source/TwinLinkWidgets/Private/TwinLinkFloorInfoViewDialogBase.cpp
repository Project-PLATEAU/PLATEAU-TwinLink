// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFloorInfoViewDialogBase.h"
#include "TwinLinkCommon.h"

#include "TwinLinkFloorInfo.h"

void UTwinLinkFloorInfoViewDialogBase::NativeConstruct() {
    Super::NativeConstruct();

    //...
}

void UTwinLinkFloorInfoViewDialogBase::NativeDestruct() {
    //...

    Super::NativeDestruct();
}

void UTwinLinkFloorInfoViewDialogBase::Setup(UObject* InData) {
    Data = Cast<UTwinLinkFloorInfo>(InData);
    check(Data.IsValid());

    // ここでデータが変更された際のイベントを登録する
}

UTwinLinkFloorInfo* UTwinLinkFloorInfoViewDialogBase::GetData() const {
    check(Data.IsValid());
    return Data.Get();
}
