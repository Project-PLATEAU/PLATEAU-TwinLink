// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "TwinLinkUrbanPlanningDataAsset.generated.h"

/**
 * 都市計画決定情報定義
 */
USTRUCT()
struct FTwinLinkUrbanPlanningFunction : public FTableRowBase {
    GENERATED_BODY()
public:
    /** 用途 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString FunctionName;
    /** カラー **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FColor FunctionColor;
public:
    FTwinLinkUrbanPlanningFunction() {
        FunctionName = FString();
        FunctionColor = FColor();
    }
};

/**
 * 都市計画決定情報データアセット
 */
UCLASS()
class TWINLINK_API UTwinLinkUrbanPlanningDataAsset : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    /** 都市計画決定情報の定義リスト **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinLink")
        TArray<FTwinLinkUrbanPlanningFunction> UrbanPlanningFunctions;
};
