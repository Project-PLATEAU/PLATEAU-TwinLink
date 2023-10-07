// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWidgetSetup.h"
#include "Kismet/GameplayStatics.h"

#include "TwinLinkCommon.h"

// Widget

#include "TwinLinkScrollBoxBase.h"

// TwinLinkシステム

#include "TwinLinkViewPointSystem.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkFloorInfoSystem.h"

#include "TwinLinkViewPointItem.h"
#include "TwinLinkFacilityItem.h"

// 
#include "TwinLinkWorldViewer.h"


void UTwinLinkWidgetSetup::SetupViewPointList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl) {
    auto ViewPointSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkViewPointSystem>();
    check(ViewPointSys.IsValid());
    auto Collection = ViewPointSys.Get()->GetViewPointCollection().Get();
    check(Collection != nullptr);

    check(ElementImpl);

    Widget->Setup(Collection, ElementImpl);

    // WorldViewerのクリックイベントに地物クリックの処理を追加
    TWeakObjectPtr<ATwinLinkWorldViewer> WorldViewer = ATwinLinkWorldViewer::GetInstance(Widget->GetWorld());
    if (WorldViewer == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("failed get world viewer"));
        return;
    }

    // クリック時に
    WorldViewer->EvOnClickedFacility.AddLambda([](FHitResult HitResult) {

        });

}

void UTwinLinkWidgetSetup::SetupFacilityInfoList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl) {
    auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    auto Collection = FacilityInfoSys.Get()->GetFacilityInfoCollection().Get();
    check(Collection != nullptr);

    check(ElementImpl);

    Widget->Setup(Collection, ElementImpl);
}
void UTwinLinkWidgetSetup::SetupFloorInfoList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl) {
    UTwinLinkObservableCollection* Collection =
        GetSelectedFloorInfoCollection();

    Widget->Setup(Collection, ElementImpl);

}

UTwinLinkObservableCollection* UTwinLinkWidgetSetup::GetSelectedFloorInfoCollection() {
    auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(FacilityInfoSys.IsValid());

    FString Key = FacilityInfoSys->GetKeyBySelectedFloor();
    if (Key.IsEmpty()) {
        Key = TEXT("dummy_key");
    }

    auto Collection = FacilityInfoSys.Get()->GetFloorInfoCollection(Key).Get();
    check(Collection != nullptr);

    return Collection;
}
