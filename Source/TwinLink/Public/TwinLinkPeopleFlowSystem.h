// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "TwinLinkTickSystem.generated.h"
#include "Interfaces/IHttpRequest.h"

USTRUCT()
struct FTwinLinkPeopleFlowApiRequest
{
    
};

USTRUCT()
struct FTWinLinkPeopleFlowApiResult
{
    // リクエストが成功したかどうか
    bool bSuccess = false;
};

/**
 * 人流データのヒートマップを扱うクラス
 */
UCLASS()
class TWINLINK_API UTwinLinkPeopleFlowSystem : public UTwinLinkSubSystemBase, public FTickableGameObject
{
	GENERATED_BODY()
public:

    UDELEGATE(BlueprintAuthorityOnly)
        DECLARE_MULTICAST_DELEGATE_OneParam(OnReceivedPeopleFlowResponseDelegate, const FTWinLinkPeopleFlowApiResult&);

    UFUNCTION(BlueprintCallable)
        void Request(const FTwinLinkPeopleFlowApiRequest& Req);
    OnReceivedPeopleFlowResponseDelegate OnReceivedPeopleFlowResponse;
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
