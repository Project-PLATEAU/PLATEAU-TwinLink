// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TwinLinkPlayerController.generated.h"

/**
 * TwinLinkで使用するプレイヤーコントローラー
 * とりあえず初期設定用に作成
 */
UCLASS()
class TWINLINK_API ATwinLinkPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
    /*
     * @brief : PlayerControllerのカメラから, MousePosのカーソル位置に対して, FieldBbの領域を貫く線分を求める
     *        : (現状ギリギリの長さではなく簡単な計算で余裕の持った長さになっている)
     *        : 線分がFieldBbとの交点を持たない場合はnulloptが返る
     */
    static std::optional<FBox>  ScreenToWorldRayThroughBoundingBox(const APlayerController* PlayerController, const FVector2D& ScreenPos, const FBox& FieldBb, float StartOffsetFromCamera = 0.f);

    /*
     * @brief : PlayerController->GetMousePositionをoptional<FVector2D>で返す
     */
    static std::optional<FVector2D> GetMousePosition(const APlayerController* PlayerController);
public:
    ATwinLinkPlayerController();

};
