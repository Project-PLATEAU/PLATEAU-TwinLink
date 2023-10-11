// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "Interfaces/IHttpRequest.h"
#include "TwinLinkPeopleFlowSystem.generated.h"

struct FPLATEAUGeoReference;

USTRUCT(BlueprintType)
struct FTwinLinkSpatialId {
    GENERATED_BODY();
public:
    int X;
    int Y;
    int F; // 高さ
    int Z; // ズームレベル
    bool bTwoDimension; // 高さを無視するかどうか

    FString AsString() const {
        return FString::Printf(TEXT("%d/%d/%d/%d/%d"), Z, F, X, Y);
    }
};

USTRUCT(BlueprintType)
struct FTwinLinkPeopleFlowApiRequest {
    GENERATED_BODY();
public:
    // 空間Idのリスト
    TArray<FTwinLinkSpatialId> SpatialIds;
    // 時刻
    FDateTime DateTime;
};
USTRUCT(BlueprintType)
struct FTwinLinkPopulationValue {
    GENERATED_BODY();
public:
    // タイムスタンプ
    FDateTime TimeStamp;
    // 人流度
    int PeopleFlow;
};

USTRUCT(BlueprintType)
struct FTwinLinkPopulationData {
    GENERATED_BODY();
public:
    // 対応する空間ID
    FTwinLinkSpatialId SpatialId;
    FString Type;
    TArray<FTwinLinkPopulationValue> Values;
};


USTRUCT(BlueprintType)
struct FTWinLinkPeopleFlowApiResult {
    GENERATED_BODY();
public:
    // リクエストが成功したかどうか
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
