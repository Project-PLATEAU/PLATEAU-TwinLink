// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"

struct TwinLinkNavSystemFindPathInfo {
    // 検索結果
    FPathFindingResult PathFindResult;
    // 高さチェックを行った後のパスポイント
    TArray<FVector> HeightCheckedPoints = {};
    int32_t NowHeightCheckIndex = 0;

    TwinLinkNavSystemFindPathInfo(FPathFindingResult&& Result);
    TwinLinkNavSystemFindPathInfo() = default;

    // リクエスト検索中
    bool IsRequesting() const;
    bool IsValid() const;
    bool IsSuccess() const;
    void Update(const UWorld* World, ECollisionChannel Channel, float MinZ, float MaxZ, int32_t CheckNum);

};