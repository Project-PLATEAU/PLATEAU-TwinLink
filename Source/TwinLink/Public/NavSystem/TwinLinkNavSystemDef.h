// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "FTwinLinkEnumT.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"

/*
 * @brief : パス検索しているときに現在開始地点/目的地点のどちらかを表しているか
 */

UENUM(BlueprintType)
enum class TwinLinkNavSystemPathPointType : uint8 {
    Start   UMETA(DisplayName = "Start"),
    Dest    UMETA(DisplayName = "Dest"),
    Max    UMETA(DisplayName = "Max"),
    Undefined UMETA(DisplayName = "Undefined")
};
using TwinLinkNavSystemPathPointTypeT = FTwinLinkEnumT<TwinLinkNavSystemPathPointType>;
/*
 * @brief : ナビメッシュ表示のモードタイプ
 */
UENUM(BlueprintType)
enum class TwinLinkNavSystemMode : uint8 {
    FindPathAnyPoint   UMETA(DisplayName = "FindPathAnyPoint"),
    FindPathBuilding   UMETA(DisplayName = "FindPathBuilding"),
    Max    UMETA(DisplayName = "Max"),
    Undefined UMETA(DisplayName = "Undefined")
};
using TwinLinkNavSystemModeT = FTwinLinkEnumT<TwinLinkNavSystemMode>;

/*
 * @brief : ナビメッシュのパスポイントの状態を表す
 */
UENUM(BlueprintType)
enum class TwinLinkNavSystemPathLocatorState : uint8 {
    // 有効な状態
    Valid              UMETA(DisplayName = "Valid"),
    // 壁にくっついている
    OnWall             UMETA(DisplayName = "OnWall"),
    // ナビメッシュの外
    OutsideNavMesh     UMETA(DisplayName = "OutsideNavMesh"),
    Max    UMETA(DisplayName = "Max"),
    Undefined UMETA(DisplayName = "Undefined")
};

UENUM(BlueprintType)
enum class TwinLinkNavSystemMoveType : uint8 {
    // 歩き
    Walk        UMETA(DisplayName = "Walk"),
    // 車
    Car         UMETA(DisplayName = "Car"),
    Max         UMETA(DisplayName = "Max"),
    Undefined   UMETA(DisplayName = "Undefined")
};
using TwinLinkNavSystemMoveTypeT = FTwinLinkEnumT<TwinLinkNavSystemMoveType>;