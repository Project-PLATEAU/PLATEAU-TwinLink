// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "TwinLinkSmartPolePairDataAsset.generated.h"

/**
 * 単位と表記のペア
 */
USTRUCT()
struct FTwinLinkSmartPolePairData : public FTableRowBase {
    GENERATED_BODY()
public:
    /** スマートポールデータの単位 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Unit;
    /** 表記の項目 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString DescriptionSection;
    /** 表記の単位 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString DescriptionUnit;
};

/**
 * 単位情報
 */
UCLASS()
class TWINLINK_API UTwinLinkSmartPolePairDataAsset : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    /** 単位と表記のペアのリスト **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinLink")
        TArray<FTwinLinkSmartPolePairData> SmartPoleUnitPair;
};
