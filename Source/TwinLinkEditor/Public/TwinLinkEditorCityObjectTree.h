// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EditorLevelLibrary.h"
#include "TwinLinkEditorCityObjectTreeParam.h"

#include "TwinLinkEditorNavSystemParam.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "TwinLinkEditorCityObjectTree.generated.h"
class APLATEAUInstancedCityModel;
class ANavMeshBoundsVolume;
/**
 * @brief : エディタ側のナビメッシュ関係を管理するクラス
 *        : `PLATEAUモデルからナビメッシュの生成等
 */
UCLASS()
class TWINLINKEDITOR_API UTwinLinkEditorCityObjectTree : public UEditorLevelLibrary {
    GENERATED_BODY()
public:
    /*
     * 空間IDの八分木生成
     */
    UFUNCTION(BlueprintCallable, Category = "Nav")
        static void CreateCityObjectTree(UEditorActorSubsystem* Editor, UWorld* World, APLATEAUInstancedCityModel* CityModel, UTwinLinkEditorCityObjectTreeParam* Param);

};