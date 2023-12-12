// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFloorInfoAddDialogBase.h"

#include "Kismet/GameplayStatics.h"
#include "TwinLinkCommon.h"

#include "TwinLinkWorldViewer.h"
#include "TwinLinkFloorInfoSystem.h"


void UTwinLinkFloorInfoAddDialogBase::NativeConstruct() {
    UTwinLinkWidgetBase::NativeConstruct();
}

void UTwinLinkFloorInfoAddDialogBase::NativeDestruct() {
    UTwinLinkWidgetBase::NativeDestruct();
}

void UTwinLinkFloorInfoAddDialogBase::Setup(const FVector& InLocationVector, const FVector2D& InUV) {
    Location = InLocationVector;
    UV = InUV;
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    // 施設カテゴリ群を設定する
    SyncCategoryCollectionWidget(
        InfoSys.Get()->GetCategoryDisplayNameCollection());

}

bool UTwinLinkFloorInfoAddDialogBase::CheckAddableFloorInfo(
    const FString& InName,
    const FString& InCategory,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {

    // 施設カテゴリ群を設定する
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    return InfoSys->CheckAddableFloorInfo(
        InName,
        InCategory,
        Location,
        UV,
        InImageFileName,
        InGuideText,
        InOpningHoursText);
}

void UTwinLinkFloorInfoAddDialogBase::AddFloorInfo(const FString& InName, const FString& InCategory, const FString& InImageFileName, const FString& InGuideText, const FString& InOpningHoursText) {
    // 施設カテゴリ群を設定する
    const auto InfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(InfoSys.IsValid());

    const auto Key = InfoSys->GetKeyBySelectedFloor();
    const auto SystemCategory = InfoSys->ConvertDisplayCategoryNameToSystemCategory(InCategory);
    check(Key.IsEmpty() == false);

    check(InfoSys->CheckAddableFloorInfo(
        InName,
        InCategory,
        Location,
        UV,
        InImageFileName,
        InGuideText,
        InOpningHoursText));

    InfoSys->AddFloorInfo(
        Key,
        InName,
        SystemCategory,
        Location,
        UV,
        InImageFileName,
        InGuideText,
        InOpningHoursText);
}

