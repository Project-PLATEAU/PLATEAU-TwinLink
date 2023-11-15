// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorLevelLibrary.h"
#include "TwinLinkEditorActorLib.generated.h"

/**
 * エディタで行うTwinLinkのアクターに向けた操作をまとめたライブラリ
 */
UCLASS()
class TWINLINKEDITOR_API UTwinLinkEditorActorLib : public UEditorLevelLibrary
{
	GENERATED_BODY()

public:
    /**
     * @brief APLATEAUInstancedCityModelに対して設定を行う
     * @param CityModel APLATEAUInstancedCityModelへの参照
     * @return 成否
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    static bool SetupCityModel(UObject* CityModel);
	
};
