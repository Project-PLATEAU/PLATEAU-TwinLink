// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TwinLinkSubSystemBase.generated.h"

/**
 * TwinLinkのサブシステムが継承するクラス
 * 共通機能を実装する
 */
UCLASS(Abstract)
class TWINLINK_API UTwinLinkSubSystemBase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

};
