// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkPlayerController.h"

#include "Kismet/GameplayStatics.h"

std::optional<FBox> ATwinLinkPlayerController::ScreenToWorldRayThroughBoundingBox(
    const APlayerController* PlayerController, const FVector2D& ScreenPos, const FBox& FieldBb,
    float StartOffsetFromCamera)
{
    if (!PlayerController)
        return std::nullopt;
    FVector WorldPosition, WorldDirection;
    if (UGameplayStatics::DeprojectScreenToWorld(PlayerController, ScreenPos, WorldPosition, WorldDirection)) {
        // カメラ位置からBoxの中心点 + Boxの対角線の長さがあれば必ず貫くことはできる
        const auto ToCenterVec = FieldBb.GetCenter() - WorldPosition;
        const auto Length = ToCenterVec.Length() + (FieldBb.Max - FieldBb.Min).Length();
        return FBox(WorldPosition + WorldDirection * StartOffsetFromCamera, WorldPosition + WorldDirection * Length);
    }
    return std::nullopt;
}

std::optional<FVector2D> ATwinLinkPlayerController::GetMousePosition(const APlayerController* PlayerController)
{
    if (!PlayerController)
        return std::nullopt;
    float MouseX, MouseY;
    if (PlayerController->GetMousePosition(MouseX, MouseY)) {
        return FVector2D(MouseX, MouseY);
    }
    return std::nullopt;
}

ATwinLinkPlayerController::ATwinLinkPlayerController() {
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableTouchEvents = true;
    bEnableMotionControls = true;
    bEnableTouchEvents = true;
}
