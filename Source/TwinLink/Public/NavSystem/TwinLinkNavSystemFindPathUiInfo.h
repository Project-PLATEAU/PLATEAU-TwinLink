// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkFacilityInfo.h"
#include "UObject/Object.h"
#include "TwinLinkNavSystemFindPathUiInfo.generated.h"


USTRUCT(BlueprintType)
struct FTwinLinkNavSystemFindPathUiInfo {
    GENERATED_BODY()
public:
    // 有効な情報かどうか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        bool bIsValid = false;

    // 移動にかかる時間[s]
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        float TimeSec = 0.f;

    // 移動距離[m]
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        float LengthMeter = 0.f;

    // UIを表示すべきスクリーン位置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FVector2D ScreenPos;

    FTwinLinkNavSystemFindPathUiInfo()
        : bIsValid(false) {
    }

    FTwinLinkNavSystemFindPathUiInfo(const float Meter, const float Sec, const FVector2D Screen)
        : bIsValid(true)
        , TimeSec(Sec)
        , LengthMeter(Meter)
        , ScreenPos(Screen) {
    }
};

USTRUCT(BlueprintType)


struct FTwinLinkNavSystemBuildingInfo {
    GENERATED_BODY()
public:
    // 建物情報
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TWeakObjectPtr<UTwinLinkFacilityInfo> FacilityInfo;
};