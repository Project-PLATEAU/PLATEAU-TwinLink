// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnumT.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"

/*
 * @brief : パス検索しているときに現在開始地点/目的地点のどちらかを阿波らしているか
 */

UENUM(BlueprintType)
enum class NavSystemPathPointType : uint8 {
    Start   UMETA(DisplayName = "Start"),
    Dest    UMETA(DisplayName = "Dest"),
    Max    UMETA(DisplayName = "Max"),
    Undefined UMETA(DisplayName = "Undefined")
};
using NavSystemPathPointTypeT = EnumT<NavSystemPathPointType>;

/*
 * @brief : ナビメッシュ表示のモードタイプ
 */
UENUM(BlueprintType)
enum class NavSystemMode : uint8 {
    FindPathAnyPoint   UMETA(DisplayName = "FindPathAnyPoint"),
    FindPathBuilding   UMETA(DisplayName = "FindPathBuilding"),
    Max    UMETA(DisplayName = "Max"),
    Undefined UMETA(DisplayName = "Undefined")
};
using NavSystemModeT = EnumT<NavSystemMode>;