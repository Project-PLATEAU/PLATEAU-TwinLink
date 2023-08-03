// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EditorLevelLibrary.h"
#include <NavMesh/NavMeshBoundsVolume.h>
#include <optional>
#include "TwinLinkEditorNavSystem.generated.h"
class APLATEAUInstancedCityModel;
class ANavMeshBoundsVolume;
/**
 * 
 */
UCLASS()
class TWINLINKEDITOR_API UTwinLinkEditorNavSystem : public UEditorLevelLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Nav")
	static void MakeNavMesh(UWorld* world);

	UFUNCTION(BlueprintCallable, Category = "Nav")
	static void SetCanEverAffectNavigationAllActors(UWorld* World, bool Relevant);
};
