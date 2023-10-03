// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkPeopleFlowSystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UTwinLinkPeopleFlowSystem::Request(const FTwinLinkPeopleFlowApiRequest& Req) {
    // https://dev.classmethod.jp/articles/unrealengine5-http-api-call/
    const FString Url = "";
    const FString Verb = "POST";


    const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UTwinLinkPeopleFlowSystem::OnResponseReceived);

    // パラメータ設定
    Request->SetURL(Url);
    Request->SetVerb(Verb);

    Request->ProcessRequest();
}

void UTwinLinkPeopleFlowSystem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
    bool bConnectionSuccessfully) {
    if (!bConnectionSuccessfully)
        return;

    // JSONオブジェクト格納用変数初期化
    TSharedPtr<FJsonObject> ResponseObj = MakeShareable(new FJsonObject());
    // 文字列からJSONを読み込むためのReader初期化
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    // 文字列からJSONオブジェクトへデシリアライズ
    if (!FJsonSerializer::Deserialize(Reader, ResponseObj))
        return;

    // UEでログ出力（APIから受け取った文字列そのまま）
    UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
}
