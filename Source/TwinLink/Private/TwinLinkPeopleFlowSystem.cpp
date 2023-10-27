// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkPeopleFlowSystem.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Interfaces/IHttpResponse.h"

FTWinLinkPeopleFlowApiResult FTWinLinkPeopleFlowApiResult::Error()
{
    FTWinLinkPeopleFlowApiResult Ret;
    Ret.bSuccess = false;         
    return Ret;
}

void UTwinLinkPeopleFlowSystem::Request(const FTwinLinkPeopleFlowApiRequest& Req) {
    // https://dev.classmethod.jp/articles/unrealengine5-http-api-call/
    const FString Url = "http://localhost:3000/population";
    const FString Verb = "GET";


    const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UTwinLinkPeopleFlowSystem::OnResponseReceived);

    // パラメータ設定
    Request->SetURL(Url);
    FJsonObject Json;
    // https://synesthesias.atlassian.net/wiki/spaces/plateaubimkukanid/pages/174751754
    // https://eukarya.notion.site/API-5587dd0c756c44d6bd697b4089bc366b
    // リクエストのjsonフォーマットはない為, 
    TArray<FString> SpatialIds;
    for (auto& Id : Req.SpatialIds)
        SpatialIds.Add(Id.StringZFXY());
    const auto Ids = FString::Join(SpatialIds, TEXT(","));
    const auto Time = Req.DateTime.ToFormattedString(TEXT("yyyy-MM-ddThh:mm:ss"));
    const auto Query = FString::Printf(TEXT("spatialId=\"%s\"&time=\"%s\""), *Ids, *Time);
    Request->SetURL(FString::Printf(TEXT("%s?%s"), *Url, *Query));
    Request->SetVerb(Verb);
    Request->ProcessRequest();
}

void UTwinLinkPeopleFlowSystem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
    bool bConnectionSuccessfully) {
    auto Result = ParseResponse(Response, bConnectionSuccessfully);
    OnReceivedPeopleFlowResponse.Broadcast(Result);
    UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
}

FTWinLinkPeopleFlowApiResult UTwinLinkPeopleFlowSystem::ParseResponse(const FHttpResponsePtr& Response,
    bool bConnectionSuccessfully)
{
    if (!bConnectionSuccessfully)
        return FTWinLinkPeopleFlowApiResult::Error();

    // JSONオブジェクト格納用変数初期化
    TSharedPtr<FJsonObject> ResponseObj = MakeShareable(new FJsonObject());
    // 文字列からJSONを読み込むためのReader初期化
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

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
