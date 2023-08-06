// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <NavMesh/NavMeshBoundsVolume.h>
#include <NavMesh/NavMeshBoundsVolume.h>
#include <optional>
#include <NavigationSystemTypes.h>
#include "TwinLinkNavSystem.generated.h"
class APLATEAUInstancedCityModel;
class ANavMeshBoundsVolume;
/**
 * 
 */
UCLASS()
class TWINLINK_API ATwinLinkNavSystem : public AActor
{
	GENERATED_BODY()
public:
    // パス検索
    void RequestFindPath(FVector Start, FVector End);

public:
};
