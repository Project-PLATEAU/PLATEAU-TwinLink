// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWalkModePanelBase.h"
#include "TwinLinkCommon.h"
#include "TwinLinkWorldViewer.h"

void UTwinLinkWalkModePanelBase::NativeConstruct() {
    Super::NativeConstruct();
    //...

    Setup();
}

void UTwinLinkWalkModePanelBase::NativeDestruct() {

    //...
    Super::NativeDestruct();
}

void UTwinLinkWalkModePanelBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
    Super::Tick(MyGeometry, InDeltaTime);
    //...
    
    TObjectPtr<ATwinLinkWorldViewer> Viewer =
        Cast<ATwinLinkWorldViewer>(ATwinLinkWorldViewer::GetInstance(GetWorld()));

    if (Viewer == nullptr)
        return;

    switch (CurrentMovement) {
    case ETwinLinkWalkModePanelBaseInputKey::Q:
        Viewer->MoveUp(-1);
        break;
    case ETwinLinkWalkModePanelBaseInputKey::E:
        Viewer->MoveUp(1);
        break;
    case ETwinLinkWalkModePanelBaseInputKey::W:
        Viewer->MoveForward(1);
        break;
    case ETwinLinkWalkModePanelBaseInputKey::S:
        Viewer->MoveForward(-1);
        break;
    case ETwinLinkWalkModePanelBaseInputKey::A:
        Viewer->MoveRight(-1);
        break;
    case ETwinLinkWalkModePanelBaseInputKey::D:
        Viewer->MoveRight(1);
        break;
    }
}

void UTwinLinkWalkModePanelBase::Setup() {

    TObjectPtr<ATwinLinkWorldViewer> Viewer =
        Cast<ATwinLinkWorldViewer>(ATwinLinkWorldViewer::GetInstance(GetWorld()));
    Viewer->EvOnChangedViewMode.AddLambda([this](ETwinLinkViewMode ViewMode) {
        if (ETwinLinkViewMode::ManualWalk == ViewMode) {
            OnChangedWalkMode(true);
        }
        else {
            OnChangedWalkMode(false);
        }
        });

    CurrentMovement = ETwinLinkWalkModePanelBaseInputKey::_Undefind;

    //...
    SetupWBP();

}

bool UTwinLinkWalkModePanelBase::IsActiveWalkMode() const {
    TObjectPtr<ATwinLinkWorldViewer> Viewer =
        Cast<ATwinLinkWorldViewer>(ATwinLinkWorldViewer::GetInstance(GetWorld()));

    if (Viewer == nullptr)
        return false;
    return Viewer->IsWalkMode();
}

void UTwinLinkWalkModePanelBase::StandbyClick() {
    if (EvOnAnyObjectClickedHnd.IsValid() == false)
        BindClickEvnet();

    bIsReadyWalkMode = true;

    return;
}

void UTwinLinkWalkModePanelBase::CancelStandbyClick() {
    bIsReadyWalkMode = false;
    return;
}

void UTwinLinkWalkModePanelBase::SetMovementSpeedMultiply(float Multiply) {
    if (IsActiveWalkMode()) {
        TObjectPtr<ATwinLinkWorldViewer> Viewer =
            Cast<ATwinLinkWorldViewer>(ATwinLinkWorldViewer::GetInstance(GetWorld()));

        if (Viewer == nullptr)
            return;

        Viewer->SetWorldViewerMovementMode(1, Multiply); // 定数修正
    }
}

void UTwinLinkWalkModePanelBase::DeactiveWalkMode() {
    TObjectPtr<ATwinLinkWorldViewer> Viewer =
        Cast<ATwinLinkWorldViewer>(ATwinLinkWorldViewer::GetInstance(GetWorld()));

    if (Viewer == nullptr)
        return;

    Viewer->ActivateAutoViewControlButton(ETwinLinkViewMode::FreeAutoView);
    Viewer->SetWorldViewerMovementMode(0, 1.0);

    return;
}

void UTwinLinkWalkModePanelBase::BeginMoveViewer(ETwinLinkWalkModePanelBaseInputKey Key) {
    CurrentMovement = Key;
}

void UTwinLinkWalkModePanelBase::EndMoveViewer() {
    CurrentMovement = ETwinLinkWalkModePanelBaseInputKey::_Undefind;
}

bool UTwinLinkWalkModePanelBase::BindClickEvnet() {
    TObjectPtr<ATwinLinkWorldViewer> Viewer =
        Cast<ATwinLinkWorldViewer>(ATwinLinkWorldViewer::GetInstance(GetWorld()));

    if (Viewer == nullptr)
        return false;

    EvOnAnyObjectClickedHnd = Viewer->EvOnAnyObjectClicked.AddLambda([this, Viewer](const FHitResult& Hit) {
        if (bIsReadyWalkMode == false)
            return;

        bIsReadyWalkMode = false;

        const auto CurrentRotation = Viewer->GetNowCameraRotationOrDefault();
        const auto TargetRotation = FRotator(0, CurrentRotation.Yaw, 0);
        Viewer->Deploy(Hit.ImpactPoint, TargetRotation);
        Viewer->ActivateAutoViewControlButton(ETwinLinkViewMode::ManualWalk);
        Viewer->SetWorldViewerMovementMode(1, 1.0); // 定数修正
        });

    return true;
}
