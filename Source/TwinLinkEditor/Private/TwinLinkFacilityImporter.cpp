// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityImporter.h"

#include "AssetSelection.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUInstancedCityModel.h"

void UTwinLinkFacilityImporter::Activate() {
    const auto Message = DoImport();

    if (!Message.IsEmpty())
        Failed.Broadcast(Message);

    FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
        while (true) {
            FPlatformProcess::Sleep(1.0f);
            if (Loader == nullptr) {
                Failed.Broadcast(TEXT("インポートに失敗しました。"));
                return;
            }
            if (Loader->Phase == ECityModelLoadingPhase::Finished) {
                Completed.Broadcast(TEXT("インポートが完了しました。"));
                return;
            }
        }
        }, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
}

UTwinLinkFacilityImporter* UTwinLinkFacilityImporter::ImportAsync(UObject* WorldContextObject, APLATEAUInstancedCityModel* CityModel, const FString& GmlPath) {
    const auto Node = NewObject<UTwinLinkFacilityImporter>();
    Node->RegisterWithGameInstance(WorldContextObject);
    Node->CityModel = CityModel;
    Node->GmlPath = GmlPath;
    return Node;
}

FString UTwinLinkFacilityImporter::DoImport() {
    const auto OriginalLoader = CityModel->Loader;

    if (OriginalLoader == nullptr)
        return TEXT("PLATEAUCityModelLoaderが見つかりません。");

    if (OriginalLoader->Phase == ECityModelLoadingPhase::Start || OriginalLoader->Phase == ECityModelLoadingPhase::Cancelling)
        return TEXT("3D都市モデルの読み込みが完了していません。");

    const auto EmptyActorAssetData = FAssetData(APLATEAUCityModelLoader::StaticClass());
    const auto EmptyActorAsset = EmptyActorAssetData.GetAsset();
    const auto Actor = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
    Loader = Cast<APLATEAUCityModelLoader>(Actor);

    Loader->ClientPtr = std::make_shared<plateau::network::Client>("", "");
    Loader->GeoReference = OriginalLoader->GeoReference;
    Loader->Source = OriginalLoader->Source;
    Loader->bImportFromServer = OriginalLoader->bImportFromServer;

    Loader->Phase = ECityModelLoadingPhase::Idle;

    Loader->LoadGmlAsync(GmlPath);

    return "";
}
