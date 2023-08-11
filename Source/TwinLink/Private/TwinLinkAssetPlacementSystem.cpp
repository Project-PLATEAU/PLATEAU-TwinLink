// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAssetPlacementSystem.h"
#include "TwinLinkCommon.h"
#include "TwinLinkTickSystem.h"

#include "TwinLinkPersistentPaths.h"

#include "TwinLinkCSVExporter.h"
#include "TwinLinkCSVImporter.h"
#include "TwinLinkCSVContents.h"

#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"

void UTwinLinkAssetPlacementSystem::Initialize(FSubsystemCollectionBase& Collection) {
    Collection.InitializeDependency<UTwinLinkTickSystem>();

    Filepath = TwinLinkPersistentPaths::CreateViewPointFilePath(TEXT("AssetPlacementInfo.csv"));
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("AssetPlacementInfo File Path : %s"), *Filepath);

    AssetPlacementInfoCollection = NewObject<UTwinLinkAssetPlacementInfoCollection>();
    ImportAssetPlacementInfo();
}

void UTwinLinkAssetPlacementSystem::Deinitialize() {
}

void UTwinLinkAssetPlacementSystem::ExportAssetPlacementInfo() {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink ExportAssetPlacementInfo : %s"), *Filepath);

    TwinLinkCSVExporter CSVExporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    CSVExporter.SetHeaderContents(
        CSVContents.CreateHeaderContents(
            TEXT("preset_id, x, y, z, roll, pitch, yaw")));

    for (const auto& AssetPlacementInfo : *AssetPlacementInfoCollection) {
        const auto& Val = AssetPlacementInfo.Value;
        const auto ID = Val->GetPresetID();
        const auto Pos = Val->GetPosition();
        const auto Rot = Val->GetRotationEuler();
        const auto StringBuf =
            CSVContents.CreateBodyContents(
                FString::Printf(TEXT("%d, %f, %f, %f, %f, %f, %f"),
                    ID,
                    Pos.X, Pos.Y, Pos.Z,
                    Rot.X, Rot.Y, Rot.Z));

        CSVExporter.AddBodyContents(StringBuf);
    }

    const auto bIsSuc = CSVExporter.ExportCSV(*Filepath);
    check(bIsSuc);
}

void UTwinLinkAssetPlacementSystem::ImportAssetPlacementInfo() {
    TwinLinkCSVImporter CSVImporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    const auto bIsSuc = CSVImporter.ImportCSV(*Filepath, &CSVContents);

    if (bIsSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Import CSV : %s"), *Filepath);
        return;
    }

    // データを解析して取得する
    const auto& BodyContents = CSVContents.GetBodyContents();
    AssetPlacementInfoCollection->Reset();
    for (const auto& BodyContentsElement : BodyContents) {
        for (const auto& ErrorStrDataElement : BodyContentsElement) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Log Contains: %s"), *ErrorStrDataElement);
        }

        // データを解析する
        const auto AssetPlacementInfo = NewObject<UTwinLinkAssetPlacementInfo>();
        AssetPlacementInfo->Setup(BodyContentsElement);

        // 無効なデータか
        if (AssetPlacementInfo->GetName() == TEXT("")) {
            // エラーデータを出力する デバッグ用
            for (const auto& ErrorStrDataElement : BodyContentsElement) {
                UE_TWINLINK_LOG(LogTemp, Log, TEXT("Contains invalid data : %s"), *ErrorStrDataElement);
            }
            // このデータの追加をスキップする
            continue;
        }

        // データを追加する
        AssetPlacementInfoCollection->Add(AssetPlacementInfo);
    }
}

TWeakObjectPtr<UTwinLinkObservableCollection> UTwinLinkAssetPlacementSystem::GetAssetPlacementInfoCollection() const {
    return AssetPlacementInfoCollection;
}

void UTwinLinkAssetPlacementSystem::AddAssetPlacementActor(const TObjectPtr<UTwinLinkAssetPlacementInfo> AssetPlacementInfo) {
    const auto Position = AssetPlacementInfo->GetPosition();
    const auto Rotation = FRotator(AssetPlacementInfo->GetRotationEuler().Y, AssetPlacementInfo->GetRotationEuler().Z, AssetPlacementInfo->GetRotationEuler().X);
    const auto AssetPlacementActor = SpawnAssetPlacementActor(GetAsset(AssetPlacementInfo->GetPresetID()), Position, Rotation);

    AssetPlacementActorCollection.Add(AssetPlacementInfo->GetUniqueID(), AssetPlacementActor);
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementUpdateActor(AActor* TargetActor) {
    const auto Key = AssetPlacementActorCollection.FindKey(TargetActor);

    if (!Key) {
        return;
    }

    const auto AssetPlacementInfo = NewObject<UTwinLinkAssetPlacementInfo>();

    AssetPlacementInfo->Setup(0, TargetActor->GetActorLocation(), FVector(TargetActor->GetActorRotation().Roll, TargetActor->GetActorRotation().Pitch, TargetActor->GetActorRotation().Yaw));

    AssetPlacementInfoCollection->Update(*Key, AssetPlacementInfo);

    ExportAssetPlacementInfo();
}

AActor* UTwinLinkAssetPlacementSystem::SpawnAssetPlacementActor(const TObjectPtr<UStaticMesh> Soil, const FVector& Location, const FRotator& Rotation) {
    const auto AssetPlacementActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
    AssetPlacementActor->SetMobility(EComponentMobility::Movable);
    AssetPlacementActor->SetActorLocation(Location);
    AssetPlacementActor->SetActorRotation(Rotation);
    const auto MeshComponent = AssetPlacementActor->GetStaticMeshComponent();
    if (MeshComponent) {
        MeshComponent->SetStaticMesh(Soil);
    }

    return AssetPlacementActor;
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementRegistSoilTest(UStaticMesh* Soil) {
    TestAssetSoil = Soil;

    //TODO　確認用処理
    for (const auto& AssetPlacementInfo : *AssetPlacementInfoCollection) {
        const auto& Val = AssetPlacementInfo.Value;
        AddAssetPlacementActor(Val);
    }
}

TObjectPtr<UStaticMesh> UTwinLinkAssetPlacementSystem::GetAsset(const int PresetID) {
    //TODO　確認用処理（テスト用アセットを返す）
    if (TestAssetSoil) {
        return TestAssetSoil;
    }
    return nullptr;
}