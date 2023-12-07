// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityInfo3DPanelBase.h"

#include "TwinLinkCommon.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkFacilityInfo.h"


void UTwinLinkFacilityInfo3DPanelBase::Setup(UObject* Info, float MinimumHeightToDisplay) {
    check(Info);

    auto CastedElement = Cast<UTwinLinkFacilityInfo>(Info);
    check(CastedElement);
    FacilityInfo = CastedElement;

    // 表示位置の計算
    const auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());

    const auto Facility = FacilityInfoSys->FindFacility(FacilityInfo);
    check(Facility.IsValid());

    const auto NavigationBounds = Facility.Get()->GetNavigationBounds();
    const auto Center = NavigationBounds.GetCenter();
    const auto ExtentZ = NavigationBounds.GetExtent().Z;

    const auto PosZ = FMath::Max(Center.Z + ExtentZ, MinimumHeightToDisplay);
    DisplayPosition = FVector(Center.X, Center.Y, PosZ);

    //DrawDebugBox(GetWorld(), Center, NavigationBounds.GetExtent(), FColor::Blue, true);
    //DrawDebugSphere(GetWorld(), DisplayPosition, 2000, 16, FColor::Green, true);
}

FVector3d UTwinLinkFacilityInfo3DPanelBase::GetDisplayPosition() const {
    return DisplayPosition;
}
