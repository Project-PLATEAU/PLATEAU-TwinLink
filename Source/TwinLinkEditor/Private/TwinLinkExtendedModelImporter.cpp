// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkExtendedModelImporter.h"

#include "AssetSelection.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUInstancedCityModel.h"
#include "Kismet/GameplayStatics.h"

void UTwinLinkExtendedModelImporter::Activate() {
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

UTwinLinkExtendedModelImporter* UTwinLinkExtendedModelImporter::ImportAsync(UObject* WorldContextObject, APLATEAUInstancedCityModel* CityModel, const double ExtensionDistance) {
    const auto Node = NewObject<UTwinLinkExtendedModelImporter>();
    Node->RegisterWithGameInstance(WorldContextObject);
    Node->CityModel = CityModel;
    Node->ExtensionDistance = ExtensionDistance;
    return Node;
}

FString UTwinLinkExtendedModelImporter::DoImport() {
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

    const auto BaseMeshCodes = OriginalLoader->MeshCodes;

    if (BaseMeshCodes.IsEmpty())
        return TEXT("");

    plateau::geometry::Extent BaseExtent(
        plateau::geometry::GeoCoordinate(999, 999, -9999),
        plateau::geometry::GeoCoordinate(-999, -999, 9999)
    );

    for (const auto BaseMeshCode : BaseMeshCodes) {
        const auto Extent = plateau::dataset::MeshCode(TCHAR_TO_UTF8(*BaseMeshCode)).getExtent();
        BaseExtent.min.latitude = FMath::Min(BaseExtent.min.latitude, Extent.min.latitude);
        BaseExtent.min.longitude = FMath::Min(BaseExtent.min.longitude, Extent.min.longitude);
        BaseExtent.max.latitude = FMath::Max(BaseExtent.max.latitude, Extent.max.latitude);
        BaseExtent.max.longitude = FMath::Max(BaseExtent.max.longitude, Extent.max.longitude);
    }

    auto Box = UPLATEAUGeoReferenceBlueprintLibrary::ProjectExtent(Loader->GeoReference, BaseExtent);
    Box.Min -= FVector::One() * ExtensionDistance;
    Box.Max += FVector::One() * ExtensionDistance;

    TSet<FString> BaseMeshCodeSet;
    for (const auto MeshCode : BaseMeshCodes) {
        BaseMeshCodeSet.Add(MeshCode);
    }

    const auto Extent = UPLATEAUGeoReferenceBlueprintLibrary::UnprojectExtent(Loader->GeoReference, Box);
    const auto MeshCodes = plateau::dataset::MeshCode::getThirdMeshes(Extent.GetNativeData());
    TArray<FString> MeshCodesToImport;

    for (const auto MeshCode : *MeshCodes) {
        const auto ThirdMeshCodeString = UTF8_TO_TCHAR(MeshCode.get().c_str());
        for (int FourthIndex = 1; FourthIndex <= 4; ++FourthIndex) {
            for (int FifthIndex = 1; FifthIndex <= 4; ++FifthIndex) {
                const auto FifthMeshCodeString = ThirdMeshCodeString + FString::FromInt(FourthIndex) + FString::FromInt(FifthIndex);
                if (!BaseMeshCodeSet.Contains(FifthMeshCodeString))
                    MeshCodesToImport.Add(FifthMeshCodeString);
            }
        }
    }

    Loader->MeshCodes = MeshCodesToImport;
    Loader->Phase = ECityModelLoadingPhase::Idle;
    const auto ImportSettings = DuplicateObject(GetMutableDefault<UPLATEAUImportSettings>(), Loader);
    for (const auto Package : Loader->ImportSettings->GetAllPackages()) {
        auto& Settings = ImportSettings->GetFeatureSettingsRef(Package);

        const auto OriginalSettings = OriginalLoader->ImportSettings->GetFeatureSettingsRef(Package);
        Settings.FallbackMaterial = OriginalSettings.FallbackMaterial;
        Settings.bAttachMapTile = OriginalSettings.bAttachMapTile;
        Settings.MapTileUrl = OriginalSettings.MapTileUrl;
        // パフォーマンス改善のため解像度を下げてインポート
        Settings.ZoomLevel = OriginalSettings.ZoomLevel - 1;

        if (Package == plateau::dataset::PredefinedCityModelPackage::Relief) {
            Settings.bImport = true;
            Settings.MinLod = 1;
            Settings.MaxLod = 1;
            Settings.MeshGranularity = EPLATEAUMeshGranularity::PerCityModelArea;
        }
        else {
            Settings.bImport = false;
        }
    }
    Loader->ImportSettings = ImportSettings;
    Loader->ImportFinishedDelegate.AddDynamic(this, &UTwinLinkExtendedModelImporter::OnPostImport);

    Loader->LoadAsync(false);

    return "";
}

void UTwinLinkExtendedModelImporter::OnPostImport() {
    TArray<AActor*> CityModels;
    UGameplayStatics::GetAllActorsOfClass(CityModel->GetWorld(), APLATEAUInstancedCityModel::StaticClass(), CityModels);
    for (const auto TargetCityModel : CityModels) {
        if (Cast<APLATEAUInstancedCityModel>(TargetCityModel)->Loader != Loader)
            continue;

        TargetCityModel->SetActorLabel(TEXT("ExtendedCityModel"));
    }
}
