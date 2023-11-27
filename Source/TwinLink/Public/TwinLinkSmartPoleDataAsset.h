// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "TwinLinkSmartPoleDataAsset.generated.h"

/**
 * スマートポールのデバイス定義
 */
USTRUCT()
struct FTwinLinkSmartPoleLinkData : public FTableRowBase {
    GENERATED_BODY()
public:
    /** デバイス名 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString DeviceName;
    /** 設置緯度 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        double Latitude;
    /** 設置経度 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        double Longitude;
    /** 設置高さ **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        double Height;
public:
    FTwinLinkSmartPoleLinkData() {
        DeviceName = FString();
        Latitude = 0.0l;
        Longitude = 0.0l;
        Height = 0.0l;
    }
};

/**
 * デバイス情報
 */
UCLASS()
class TWINLINK_API UTwinLinkSmartPoleDataAsset : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    /** スマートポールのデバイス定義リスト **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinLink")
        TArray<FTwinLinkSmartPoleLinkData> SmartPoleLinkData;
};
