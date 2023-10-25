// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once
#include "UObject/ObjectMacros.h"
#include "TwinLinkPopulationValue.generated.h"

USTRUCT(BlueprintType)
struct FTwinLinkPopulationValue {
    GENERATED_BODY();
public:
    // タイムスタンプ
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FDateTime TimeStamp;
    // 人流度
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FString Unit;
    // 人流度
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        int PeopleFlow;
};