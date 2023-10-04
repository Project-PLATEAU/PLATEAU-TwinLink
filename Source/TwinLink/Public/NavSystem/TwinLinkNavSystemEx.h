// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TwinLinkNavSystemEx.generated.h"
struct FTwinLinkNavSystemBuildingInfo;
/*
 * @brief : ナビメッシュ関係の便利関数
 */
UCLASS()
class TWINLINK_API UTwinLinkNavSystemEx : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    /*
     * @brief : パス検索可能になったときに呼ばれるイベント
     */
    UFUNCTION(BlueprintCallable)
        static bool IsValidBuilding(const FTwinLinkNavSystemBuildingInfo& Info);

    /*
     * @brief : 入口情報を返す
     */
    UFUNCTION(BlueprintCallable)
        static FVector GetFirstEntranceOrZero(const FTwinLinkNavSystemBuildingInfo& Info);

    /*
     * @brief : 表示に必要な情報を取得する
     */
    UFUNCTION(BlueprintCallable)
        static bool TryGetBuildingInfo(const FTwinLinkNavSystemBuildingInfo& Info, FString& OutBuildingName, FString& OutBuildingIconKey, FVector& OutEntrance);
};
