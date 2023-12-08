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
            TEXT("preset_id, x, y, z, roll, pitch, yaw, scale_x, scale_y, scale_z")));

    for (const auto& AssetPlacementInfo : *AssetPlacementInfoCollection) {
        const auto& Val = AssetPlacementInfo.Value;
        const auto ID = Val->GetPresetID();
        const auto Pos = Val->GetPosition();
        const auto Rot = Val->GetRotationEuler();
        const auto Sca = Val->GetScale();
        const auto StringBuf =
            CSVContents.CreateBodyContents(
                FString::Printf(TEXT("%d, %f, %f, %f, %f, %f, %f, %f, %f, %f"),
                    ID,
                    Pos.X, Pos.Y, Pos.Z,
                    Rot.X, Rot.Y, Rot.Z,
                    Sca.X, Sca.Y, Sca.Z));

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
    const auto Scale = AssetPlacementInfo->GetScale();
    TObjectPtr<AActor> AssetPlacementActor;
    const auto Preset = GetPresetAssetMesh(AssetPlacementInfo->GetPresetID());
    if (Cast<UStaticMesh>(Preset) != nullptr) {
        AssetPlacementActor = SpawnAssetPlacementActor(Cast<UStaticMesh>(Preset), Position, Rotation, Scale);
    }
    else if (Cast<UBlueprint>(Preset)) {
        auto Actor = GetWorld()->SpawnActor<AStaticMeshActor>(Cast<UBlueprint>(Preset)->GeneratedClass);
        Actor->SetMobility(EComponentMobility::Movable);
        Actor->SetActorLocation(Position);
        Actor->SetActorRotation(Rotation);
        Actor->SetActorScale3D(Scale);

        AssetPlacementActor = Actor;
    }
    else {
        TSubclassOf<class AStaticMeshActor> StaticMeshActor = TSoftClassPtr<AStaticMeshActor>(FSoftObjectPath(*Preset.GetPath())).LoadSynchronous();

        if (StaticMeshActor != nullptr) {
            auto Actor = GetWorld()->SpawnActor<AStaticMeshActor>(StaticMeshActor);
            Actor->SetMobility(EComponentMobility::Movable);
            Actor->SetActorLocation(Position);
            Actor->SetActorRotation(Rotation);
            Actor->SetActorScale3D(Scale);

            AssetPlacementActor = Actor;
        }
        else {
            return;
        }
    }

    AssetPlacementActorCollection.Add(AssetPlacementInfo->GetUniqueID(), AssetPlacementActor);
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementUpdateActor(AActor* TargetActor) {
    const auto Key = AssetPlacementActorCollection.FindKey(TargetActor);

    if (!Key) {
        return;
    }
    const auto AssetPlacementInfoOrg = AssetPlacementInfoCollection->GetFromKey(*Key);

    const auto AssetPlacementInfo = NewObject<UTwinLinkAssetPlacementInfo>();

    AssetPlacementInfo->Setup(AssetPlacementInfoOrg->Get()->GetPresetID(), TargetActor->GetActorLocation(), FVector(TargetActor->GetActorRotation().Roll, TargetActor->GetActorRotation().Pitch, TargetActor->GetActorRotation().Yaw), TargetActor->GetActorScale3D());

    AssetPlacementInfoCollection->Update(*Key, AssetPlacementInfo);

    ExportAssetPlacementInfo();
}

AActor* UTwinLinkAssetPlacementSystem::SpawnAssetPlacementActor(const TObjectPtr<UStaticMesh> Soil, const FVector& Location, const FRotator& Rotation, const FVector& Scale) {
    const auto AssetPlacementActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
    AssetPlacementActor->SetMobility(EComponentMobility::Movable);
    AssetPlacementActor->SetActorLocation(Location);
    AssetPlacementActor->SetActorRotation(Rotation);
    AssetPlacementActor->SetActorScale3D(Scale);
    const auto MeshComponent = AssetPlacementActor->GetStaticMeshComponent();
    if (MeshComponent) {
        MeshComponent->SetStaticMesh(Soil);
    }

    return AssetPlacementActor;
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementRemoveActor(AActor* TargetActor) {
    const auto Key = AssetPlacementActorCollection.FindKey(TargetActor);

    if (!Key) {
        return;
    }

    AssetPlacementActorCollection.Remove(*Key);

    const auto AssetPlacementInfoOrg = AssetPlacementInfoCollection->GetFromKey(*Key);

    AssetPlacementInfoCollection->RemoveFromKey(*Key);

    ExportAssetPlacementInfo();
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementRelocation() {
    for (const auto& AssetPlacementInfo : *AssetPlacementInfoCollection) {
        const auto& Val = AssetPlacementInfo.Value;
        if (!PresetMeshes.Contains(Val->GetPresetID())) {
            continue;
        }
        AddAssetPlacementActor(Val);
    }
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementRegistPresetAsset(const FTwinLinkAssetPlacementPresetData& Presets) {
    PresetIDs.Add(Presets.PresetID);
    TSoftObjectPtr<UObject> Asset = Presets.AssetSils.LoadSynchronous();
    //アセットのロードに失敗したらBPの読み込みを試す
    if (Asset == nullptr) {
        FString Path = Presets.AssetSils.GetLongPackageName();
        int Index = 0;
        if (Path.FindLastChar('/', Index)) {
            Path += TEXT(".") + Path.Right(Path.Len() - Index - 1) + TEXT("_C");
            TSubclassOf<class AStaticMeshActor> StaticMeshActor = TSoftClassPtr<AStaticMeshActor>(FSoftObjectPath(*Path)).LoadSynchronous();
            Asset = StaticMeshActor.Get();
        }
    }
    PresetMeshes.Add(Presets.PresetID, Asset.Get());
    PresetTextures.Add(Presets.PresetID, Presets.WidgetTexture);
}

void UTwinLinkAssetPlacementSystem::SpawnUnsettledActor(const TObjectPtr<UTwinLinkAssetPlacementInfo> AssetPlacementInfo) {
    const auto Position = AssetPlacementInfo->GetPosition();
    const auto Rotation = FRotator(AssetPlacementInfo->GetRotationEuler().Y, AssetPlacementInfo->GetRotationEuler().Z, AssetPlacementInfo->GetRotationEuler().X);
    const auto Scale = AssetPlacementInfo->GetScale();

    const auto Preset = GetPresetAssetMesh(AssetPlacementInfo->GetPresetID());
    if (Cast<UStaticMesh>(Preset) != nullptr) {
        AssetPlacementUnsettledActor = SpawnAssetPlacementActor(Cast<UStaticMesh>(Preset), Position, Rotation, Scale);
    }
    else if (Cast<UBlueprint>(Preset)) {
        auto UnsettledActor = GetWorld()->SpawnActor<AStaticMeshActor>(Cast<UBlueprint>(Preset)->GeneratedClass);
        UnsettledActor->SetMobility(EComponentMobility::Movable);
        UnsettledActor->SetActorLocation(Position);
        UnsettledActor->SetActorRotation(Rotation);
        UnsettledActor->SetActorScale3D(Scale);

        AssetPlacementUnsettledActor = UnsettledActor;
    }
    else {
        TSubclassOf<class AStaticMeshActor> StaticMeshActor = TSoftClassPtr<AStaticMeshActor>(FSoftObjectPath(*Preset.GetPath())).LoadSynchronous();

        if (StaticMeshActor != nullptr) {
            auto UnsettledActor = GetWorld()->SpawnActor<AStaticMeshActor>(StaticMeshActor);
            UnsettledActor->SetMobility(EComponentMobility::Movable);
            UnsettledActor->SetActorLocation(Position);
            UnsettledActor->SetActorRotation(Rotation);
            UnsettledActor->SetActorScale3D(Scale);

            AssetPlacementUnsettledActor = UnsettledActor;
        }
        else {
            return;
        }
    }

    AssetPlacementUnsettledActor->GetRootComponent()->SetHiddenInGame(true);

    AssetPlacementUnsettledAssetPlacementInfo = AssetPlacementInfo;

    CurrentMode = ETwinLinkAssetPlacementModes::AssetPlacement;
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementUpdateUnsettledActor(const FVector& Location) {
    if (!AssetPlacementUnsettledActor) {
        return;
    }
    if (TwinLinkAssetPlacementIsWidgetUnderMouseCursor()) {
        return;
    }

    if (AssetPlacementUnsettledActor->GetRootComponent()->bHiddenInGame) {
        AssetPlacementUnsettledActor->GetRootComponent()->SetHiddenInGame(false);
    }
    AssetPlacementUnsettledActor->SetActorLocation(Location);
}

bool UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementIsWidgetUnderMouseCursor() {
    FSlateApplication& SlateApplication = FSlateApplication::Get();
    const FWidgetPath WidgetsUnderCursor = SlateApplication.LocateWindowUnderMouse(
        SlateApplication.GetCursorPos(), SlateApplication.GetInteractiveTopLevelWindows()
    );

    if (WidgetsUnderCursor.IsValid()) {
        const FArrangedChildren::FArrangedWidgetArray& AllArrangedWidgets = WidgetsUnderCursor.Widgets.GetInternalArray();
        bool FoundRoot = false;
        for (const auto& AllArrangedWidget : AllArrangedWidgets) {
            const TSharedRef<SWidget> Widget = AllArrangedWidget.Widget;
            if (Widget->GetTypeAsString() == "SGameLayerManager") {
                FoundRoot = true;
            }
            else if (FoundRoot) {
                return true;
            }
        }
    }

    return false;
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementSettledActor() {
    const auto AssetPlacementInfo = NewObject<UTwinLinkAssetPlacementInfo>();

    AssetPlacementInfo->Setup(AssetPlacementUnsettledAssetPlacementInfo->GetPresetID(), AssetPlacementUnsettledActor->GetActorLocation(), FVector(AssetPlacementUnsettledActor->GetActorRotation().Roll, AssetPlacementUnsettledActor->GetActorRotation().Pitch, AssetPlacementUnsettledActor->GetActorRotation().Yaw), AssetPlacementUnsettledActor->GetActorScale3D());

    //コレクション追加
    AssetPlacementInfoCollection->Add(AssetPlacementInfo);
    //アクタ追加
    AssetPlacementActorCollection.Add(AssetPlacementInfo->GetUniqueID(), AssetPlacementUnsettledActor);

    ExportAssetPlacementInfo();

    AssetPlacementUnsettledActor = nullptr;
    AssetPlacementUnsettledAssetPlacementInfo = nullptr;

    CurrentMode = ETwinLinkAssetPlacementModes::AssetTransform;
}

void UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementClearUnsettledActor() {
    if (AssetPlacementUnsettledActor != nullptr) {
        AssetPlacementUnsettledActor->Destroy();
    }

    AssetPlacementUnsettledActor = nullptr;
    AssetPlacementUnsettledAssetPlacementInfo = nullptr;

    CurrentMode = ETwinLinkAssetPlacementModes::AssetTransform;
}

bool UTwinLinkAssetPlacementSystem::TwinLinkAssetPlacementIsPresetActor(AActor* Actor) {
    TArray<TObjectPtr<AActor>> Actors;
    AssetPlacementActorCollection.GenerateValueArray(Actors);
    return Actors.Contains(Actor);
}

TObjectPtr<UObject> UTwinLinkAssetPlacementSystem::GetPresetAssetMesh(const int PresetID) {
    if (PresetMeshes.Num() > 0) {
        return PresetMeshes[PresetID];
    }

    return nullptr;
}