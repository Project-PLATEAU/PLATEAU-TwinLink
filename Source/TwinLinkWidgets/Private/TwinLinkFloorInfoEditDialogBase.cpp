// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFloorInfoEditDialogBase.h"

#include "TwinLinkCommon.h"
#include "TwinLinkFloorInfoSystem.h"
#include "TwinLinkFloorInfo.h"

void UTwinLinkFloorInfoEditDialogBase::NativeConstruct() {
    UTwinLinkWidgetBase::NativeConstruct();
}

void UTwinLinkFloorInfoEditDialogBase::NativeDestruct() {
    UTwinLinkWidgetBase::NativeDestruct();
}

void UTwinLinkFloorInfoEditDialogBase::Setup(UTwinLinkFloorInfo* Info) {
    check(Info != nullptr);
    FloorInfo = Info;

    // ここでデータが更新されたら修正？
}

UTwinLinkFloorInfo* UTwinLinkFloorInfoEditDialogBase::GetData() const {
    check(FloorInfo.IsValid());
    return FloorInfo.Get();
}

FString UTwinLinkFloorInfoEditDialogBase::GetDisplayCategoryName() const {
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    const auto DisplayCategoryName =
        InfoSys->ConvertSystemCategoryToDisplayCategoryName(FloorInfo->GetCategory());
    return DisplayCategoryName;
}

TArray<FString> UTwinLinkFloorInfoEditDialogBase::GetCategoryGroup() const {
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    return InfoSys.Get()->GetCategoryDisplayNameCollection();
}

void UTwinLinkFloorInfoEditDialogBase::RequestEdit(
    const FString& InName, const FString& InCategory,
    const FString& InImageFileName, const FString& InGuide,
    const FString& InSpotInfo) {
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    check(FloorInfo.IsValid());
    
    const auto SystemCategory = InfoSys->ConvertDisplayCategoryNameToSystemCategory(InCategory);


    check(CheckAddableFloorInfo(
        InName,
        SystemCategory,
        InImageFileName,
        InGuide,
        InSpotInfo));

    // 編集を行う
    InfoSys->EditFloorInfo(
        FloorInfo,
        InName,
        SystemCategory,
        InImageFileName,
        InGuide,
        InSpotInfo
    );

    // 出力する
    InfoSys->ExportFloorInfo();
}

bool UTwinLinkFloorInfoEditDialogBase::CheckAddableFloorInfo(
    const FString& InName,
    const FString& InCategory,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {

    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());
    check(FloorInfo.IsValid());

    const auto bIsAddable = InfoSys->CheckAddableFloorInfo(
        InName,
        InCategory,
        FloorInfo->GetLocation(),
        InImageFileName,
        InGuideText,
        InOpningHoursText);
    return bIsAddable;
}
