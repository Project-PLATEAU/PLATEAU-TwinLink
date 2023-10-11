// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSpatialID.h"
#include "TwinLinkSubSystemBase.h"
#include "Interfaces/IHttpRequest.h"
#include "TwinLinkPeopleFlowSystem.generated.h"

struct FPLATEAUGeoReference;

USTRUCT(BlueprintType)
struct FTwinLinkPeopleFlowApiRequest {
    GENERATED_BODY();
public:
    // 空間Idのリスト
    UPROPERTY(EditAnywhere)
        TArray<FString> SpatialIds;
    // 時刻
    UPROPERTY(EditAnywhere)
        FDateTime DateTime;
};
USTRUCT(BlueprintType)
struct FTwinLinkPopulationValue {
    GENERATED_BODY();
public:
    // タイムスタンプ
    UPROPERTY(EditAnywhere)
        FDateTime TimeStamp;
    // 人流度
    UPROPERTY(EditAnywhere)
        FString Unit;
    // 人流度
    UPROPERTY(EditAnywhere)
        int PeopleFlow;
};

USTRUCT(BlueprintType)
struct FTwinLinkPopulationData {
    GENERATED_BODY();
public:
    // 対応する空間ID

    UPROPERTY(EditAnywhere)
        FString SpatialId;

    UPROPERTY(EditAnywhere)
        FString Type;

    UPROPERTY(EditAnywhere)
        TArray<FTwinLinkPopulationValue> Values;
};


USTRUCT(BlueprintType)
struct FTWinLinkPeopleFlowApiResult {
    GENERATED_BODY();
public:
    // リクエストが成功したかどうか
    UPROPERTY(EditAnywhere)
        bool bSuccess = false;
};

/**
 * 人流データのヒートマップを扱うクラス
 */
UCLASS()
class TWINLINK_API UTwinLinkPeopleFlowSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()
public:
    UDELEGATE(BlueprintAuthorityOnly)
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceivedPeopleFlowResponseDelegate, const FTWinLinkPeopleFlowApiResult&, Result);

    UFUNCTION(BlueprintCallable)
        void Request(const FTwinLinkPeopleFlowApiRequest& Req);
    FOnReceivedPeopleFlowResponseDelegate OnReceivedPeopleFlowResponse;
private:
    /*
     * @brief : レスポンスコールバック
     */
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectionSuccessfully);


private:
    // 最後にリクエストした時刻
    UPROPERTY()
        FDateTime LastRequestDateTime;
};
