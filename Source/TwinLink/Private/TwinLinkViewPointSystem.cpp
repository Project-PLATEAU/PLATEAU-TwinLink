// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkViewPointSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

#include "TwinLinkCommon.h"

#include "TwinLinkTickSystem.h"

#include "TwinLinkPersistentPaths.h"
#include "TwinLinkViewPointInfo.h"

#include "TwinLinkCSVExporter.h"
#include "TwinLinkCSVImporter.h"
#include "TwinLinkCSVContents.h"

#include "TwinLinkWorldViewer.h"

#include "GameFramework/CharacterMovementComponent.h"

void UTwinLinkViewPointSystem::Initialize(FSubsystemCollectionBase& Collection) {
    Collection.InitializeDependency<UTwinLinkTickSystem>();

    Filepath = TwinLinkPersistentPaths::CreateViewPointFilePath(TEXT("ViewPointInfo.csv"));
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("ViewPointInfo File Path : %s"), *Filepath);

    ViewPointInfoCollection = NewObject<UTwinLinkViewPointInfoCollection>();
    ImportViewPointInfo();
}

void UTwinLinkViewPointSystem::Deinitialize() {
}

bool UTwinLinkViewPointSystem::CheckAddableViewPointInfo(const FString& ViewPointName) {
    if (ViewPointName.IsEmpty()) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink Cant add empty name : %s"), *ViewPointName);
        return false;
    }

    const auto ApplicableTargets = ATwinLinkWorldViewer::GetInstance(GetWorld());
    // 視点情報を取得、適応する対象が存在しない
    if (ApplicableTargets == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink Not ApplicableTargets : %s"), *ViewPointName);
        return false;
    }

    return true;
}

void UTwinLinkViewPointSystem::AddViewPointInfo(const FString& ViewPointName) {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink AddViewPointInfo : %s"), *Filepath);
    const auto ApplicableTargets = ATwinLinkWorldViewer::GetInstance(GetWorld());

    FVector Position;
    FVector RotationEuler;

    // CheckAddableViewPointInfo()内で チェック済みなので正常な値が格納されている前提で記述する
    check(ApplicableTargets.IsValid());

    // 視点情報の作成

    auto P = ApplicableTargets.Get();
    Position = P->GetActorLocation();
    RotationEuler = P->GetViewRotation().Euler();
    UTwinLinkViewPointInfo* ViewPointInfo = NewObject<UTwinLinkViewPointInfo>();
    const auto bIsSuc = ViewPointInfo->Setup(ViewPointName, Position, RotationEuler);

    // CheckAddableViewPointInfo()内で チェック済みなので正常な値が格納されている前提で記述する
    check(bIsSuc);

    // 視点情報の追加
    ViewPointInfoCollection->Add(ViewPointInfo);
}

void UTwinLinkViewPointSystem::RemoveViewPointInfo(const TWeakObjectPtr<UTwinLinkViewPointInfo>& ViewPointInfo) {
    check(ViewPointInfo.IsValid());
    ViewPointInfoCollection->Remove(ViewPointInfo);
}

void UTwinLinkViewPointSystem::RequestEditName(const TWeakObjectPtr<UTwinLinkViewPointInfo>& ViewPointInfo, const FString& NewName) {
    const auto IsAddable = CheckAddableViewPointInfo(NewName);
    if (IsAddable == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Cant Add ViewPoint : %s"), *NewName);
        return;
    }

    check(ViewPointInfo.IsValid());
    const auto Pos = ViewPointInfo->GetPosition();
    const auto RotEulur = ViewPointInfo->GetRotationEuler();
    const auto IsSuc = ViewPointInfo->Setup(NewName, Pos, RotEulur);
    if (IsSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed setup ViewPoint : %s"), *NewName);
        return;
    }

}

void UTwinLinkViewPointSystem::ExportViewPointInfo() {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink ExportViewPointInfo : %s"), *Filepath);

    TwinLinkCSVExporter CSVExporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    CSVExporter.SetHeaderContents(
        CSVContents.CreateHeaderContents(
            TEXT("name, x, y, z, roll, pitch, yaw")));

    FString StringBuf;
    for (const auto& ViewPointInfo : *ViewPointInfoCollection) {
        const auto& Val = ViewPointInfo.Value;
        const auto Name = Val->GetName();
        const auto Pos = Val->GetPosition();
        const auto Rot = Val->GetRotationEuler();
        StringBuf =
            CSVContents.CreateBodyContents(
                FString::Printf(TEXT("%s,%f,%f,%f,%f,%f,%f"),
                    *(Name),
                    Pos.X, Pos.Y, Pos.Z,
                    Rot.X, Rot.Y, Rot.Z));

        CSVExporter.AddBodyContents(StringBuf);
    }

    const auto bIsSuc = CSVExporter.ExportCSV(*Filepath);
    check(bIsSuc);
}

void UTwinLinkViewPointSystem::ImportViewPointInfo() {
    TwinLinkCSVImporter CSVImporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    const auto bIsSuc = CSVImporter.ImportCSV(*Filepath, &CSVContents);

    if (bIsSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Import CSV : %s"), *Filepath);
        return;
    }

    // データを解析して取得する
    const auto& BodyContents = CSVContents.GetBodyContents();
    ViewPointInfoCollection->Reset();
    for (const auto& BodyContentsElement : BodyContents) {
        for (const auto& ErrorStrDataElement : BodyContentsElement) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Log Contains: %s"), *ErrorStrDataElement);
        }

        // データを解析する
        const auto ViewPointInfo = NewObject<UTwinLinkViewPointInfo>();
        ViewPointInfo->Setup(BodyContentsElement);

        // 無効なデータか
        if (ViewPointInfo->GetName() == TEXT("")) {
            // エラーデータを出力する デバッグ用
            for (const auto& ErrorStrDataElement : BodyContentsElement) {
                UE_TWINLINK_LOG(LogTemp, Log, TEXT("Contains invalid data : %s"), *ErrorStrDataElement);
            }
            // このデータの追加をスキップする
            continue;
        }

        // データを追加する
        ViewPointInfoCollection->Add(ViewPointInfo);
    }
}

void UTwinLinkViewPointSystem::ApplyViewPointInfo(const TWeakObjectPtr<UTwinLinkViewPointInfo>& ViewPointInfo) {
    const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
    check(ViewPointInfo.IsValid());
    if (WorldViewer == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("視点情報の適用対象が存在しない : %s"), *(ViewPointInfo.Get()->GetName()));
        return;
    }

    const auto bIsContains =
        ViewPointInfoCollection->Contains(ViewPointInfo);
    check(bIsContains);

    WorldViewer->SetLocation(ViewPointInfo.Get()->GetPosition(), FRotator::MakeFromEuler(ViewPointInfo.Get()->GetRotationEuler()));

}

TWeakObjectPtr<UTwinLinkObservableCollection> UTwinLinkViewPointSystem::GetViewPointCollection() const {
    return ViewPointInfoCollection;
}
