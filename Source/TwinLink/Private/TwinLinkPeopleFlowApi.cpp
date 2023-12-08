// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkPeopleFlowApi.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Interfaces/IHttpResponse.h"

//#define TWINLINK_USE_POST
FTWinLinkPeopleFlowApiResult FTWinLinkPeopleFlowApiResult::Error() {
    FTWinLinkPeopleFlowApiResult Ret;
    Ret.bSuccess = false;
    return Ret;
}

void UTwinLinkPeopleFlowApi::Request(const FTwinLinkPeopleFlowApiRequest& Req) {
    // https://dev.classmethod.jp/articles/unrealengine5-http-api-call/
    const FString Url = "https://plateau-spatialid-bim-api-prod-3xfxdvazpa-an.a.run.app/population";

    const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UTwinLinkPeopleFlowApi::OnResponseReceived);
    Request->SetTimeout(5);
    FJsonObject Json;
    // https://synesthesias.atlassian.net/wiki/spaces/plateaubimkukanid/pages/174751754
    // https://eukarya.notion.site/API-5587dd0c756c44d6bd697b4089bc366b
    // リクエストのjsonフォーマットはない為, 
    TArray<FString> SpatialIds;
    for (auto& Id : Req.SpatialIds)
        SpatialIds.Add(Id.StringZFXY());
    const auto Ids = FString::Join(SpatialIds, TEXT(","));
    // #NOTE : タイムゾーンは日本前提
    const auto Time = Req.DateTime.ToFormattedString(TEXT("%Y-%m-%dT%H:%M:%S+09:00"));
    constexpr bool bUsePost = true;
    if (bUsePost) {
        Request->SetVerb(TEXT("POST"));
        Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
        if (Req.bUseDateTime) {
            const auto Content = FString::Printf(TEXT("{\"spatialId\":\"%s\",\"time\":\"%s\"}"), *Ids, *Time);
            Request->SetContentAsString(Content);
        }
        else {
            const auto Content = FString::Printf(TEXT("{\"spatialId\":\"%s\"}"), *Ids);
            Request->SetContentAsString(Content);
        }
        Request->SetURL(Url);
    }
    else {
        const FString Verb = "GET";
        Request->SetVerb(TEXT("GET"));
        const auto Query = FString::Printf(TEXT("spatialId=\"%s\"&time=\"%s\""), *Ids, *Time);
        Request->SetURL(FString::Printf(TEXT("%s?%s"), *Url, *Query));
    }
    Request->ProcessRequest();
}

void UTwinLinkPeopleFlowApi::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
    bool bConnectionSuccessfully) {
    auto Result = ParseResponse(Response, bConnectionSuccessfully);
    OnReceivedPeopleFlowResponse.Broadcast(Result);
    UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
}

FTWinLinkPeopleFlowApiResult UTwinLinkPeopleFlowApi::ParseResponse(const FHttpResponsePtr& Response,
    bool bConnectionSuccessfully) {
    if (!bConnectionSuccessfully)
        return FTWinLinkPeopleFlowApiResult::Error();

    // JSONオブジェクト格納用変数初期化
    TSharedPtr<FJsonObject> ResponseObj = MakeShareable(new FJsonObject());
    // 文字列からJSONを読み込むためのReader初期化
    const auto ContentStr = Response->GetContentAsString();
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ContentStr);

    // 文字列からJSONオブジェクトへデシリアライズ
    if (!FJsonSerializer::Deserialize(Reader, ResponseObj))
        return FTWinLinkPeopleFlowApiResult::Error();

    FTWinLinkPeopleFlowApiResult Ret;
    Ret.bSuccess = true;
    // UEでログ出力（APIから受け取った文字列そのまま
    for (const auto& Data : ResponseObj->GetArrayField(TEXT("data"))) {
        auto& Json = Data->AsObject();
        FTwinLinkPopulationData D;
        D.SpatialId = FTwinLinkSpatialID::ParseZFXY(Json->GetStringField(TEXT("id")));
        D.Type = Json->GetStringField(TEXT("type"));
        for (auto& V : Json->GetArrayField(TEXT("values"))) {
            auto& JsonV = V->AsObject();
            FTwinLinkPopulationValue Val;
            FDateTime::Parse(JsonV->GetStringField("timestamp"), Val.TimeStamp);
            Val.Unit = JsonV->GetStringField(TEXT("unit"));
            Val.PeopleFlow = JsonV->GetIntegerField(TEXT("peopleFlow"));
            D.Values.Add(Val);
        }
        Ret.Populations.Add(D);
    }
    return Ret;
}
