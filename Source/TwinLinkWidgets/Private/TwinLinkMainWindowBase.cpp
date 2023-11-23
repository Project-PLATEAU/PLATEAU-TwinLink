// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkMainWindowBase.h"
#include "TwinLinkCommon.h"
#include "TwinLinkWorldViewer.h"
#include "TwinLinkFacilityInfoSystem.h"

void UTwinLinkMainWindowBase::NativeConstruct() {
    UTwinLinkWidgetBase::NativeConstruct();
    SetupMainWindow();
}

void UTwinLinkMainWindowBase::NativeDestruct() {
    FinalizeMainWindow();
    UTwinLinkWidgetBase::NativeDestruct();
}

void UTwinLinkMainWindowBase::SetupMainWindow() {
    const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
    if (WorldViewer.IsValid() == false)
        return;

    // クリック時にイベントを追加する
    EvOnClickedFacilityHnd = WorldViewer->EvOnClickedFacility.AddLambda([this](FHitResult HitResult) {
        auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
        check(FacilityInfoSys.IsValid());
        const auto FeatureID = HitResult.Component->GetName();
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("FeatureID %s"), *FeatureID);

        // 施設がクリックされた時に呼び出される
        OnClickedFacility(FeatureID);
        });

}

void UTwinLinkMainWindowBase::FinalizeMainWindow() {
    const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());

    // クリック時にイベントを削除する
    WorldViewer->EvOnClickedFacility.Remove(EvOnClickedFacilityHnd);
    EvOnClickedFacilityHnd.Reset();

}
