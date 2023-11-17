// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkPopulationData.h"
#include "TwinLinkSpatialID.h"
#include "TwinLinkSubSystemBase.h"
#include "Interfaces/IHttpRequest.h"
#include "TwinLinkPeopleFlowApi.generated.h"

struct FPLATEAUGeoReference;

USTRUCT(BlueprintType)
struct FTwinLinkPeopleFlowApiRequest {
    GENERATED_BODY();
public:
    // 空間Idのリスト
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TArray<FTwinLinkSpatialID> SpatialIds;
    // 時刻
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        FDateTime DateTime;
};

USTRUCT(BlueprintType)
struct FTWinLinkPeopleFlowApiResult {
    GENERATED_BODY();
public:
    // リクエストが成功したかどうか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base")
        TArray<FTwinLinkPopulationData> Populations;

public:
    static FTWinLinkPeopleFlowApiResult Error();
};

/**
 * 人流データのヒートマップを扱うクラス
 */
UCLASS(BlueprintType)
class TWINLINK_API UTwinLinkPeopleFlowApi : public UObject {
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

    static FTWinLinkPeopleFlowApiResult ParseResponse(const FHttpResponsePtr& Response, bool bConnectionSuccessfully);
private:
    // 最後にリクエストした時刻
    UPROPERTY()
        FDateTime LastRequestDateTime;
};
