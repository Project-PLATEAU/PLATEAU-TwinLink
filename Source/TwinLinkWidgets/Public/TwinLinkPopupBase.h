// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkPopupBase.generated.h"

/**
 * ポップアップウィジェットの基底クラス
 * 確認ポップアップ類はこれを継承する
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkPopupBase : public UTwinLinkWidgetBase
{
	GENERATED_BODY()
	
};
