// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

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
    ATwinLinkPlayerController();

};
