// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkPeopleFlowSystem.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Interfaces/IHttpResponse.h"

void UTwinLinkPeopleFlowSystem::Request(const FTwinLinkPeopleFlowApiRequest& Req) {
    // 人流データ取得API構築
    if (!Api) {
        Api = NewObject<UTwinLinkPeopleFlowApi>();
        Api->OnReceivedPeopleFlowResponse.AddDynamic(this, &UTwinLinkPeopleFlowSystem::OnReceivedPeopleFlowResponseImpl);
    }

    Api->Request(Req);
}

void UTwinLinkPeopleFlowSystem::OnReceivedPeopleFlowResponseImpl(const FTWinLinkPeopleFlowApiResult& Result) {
    OnReceivedPeopleFlowResponse.Broadcast(Result);
}
