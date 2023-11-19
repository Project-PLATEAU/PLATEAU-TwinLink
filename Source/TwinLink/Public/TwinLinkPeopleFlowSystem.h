// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkPeopleFlowApi.h"
#include "TwinLinkSubSystemBase.h"
#include "Interfaces/IHttpRequest.h"
#include "TwinLinkPeopleFlowSystem.generated.h"

struct FPLATEAUGeoReference;

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
    // 人流データレスポンス受信
    UFUNCTION()
        void OnReceivedPeopleFlowResponseImpl(const FTWinLinkPeopleFlowApiResult& Result);
private:
    // 最後にリクエストした時刻
    UPROPERTY()
        FDateTime LastRequestDateTime;

    UPROPERTY()
        UTwinLinkPeopleFlowApi* Api = nullptr;
};
