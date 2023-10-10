// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFloorInfoSystem.h"
#include "Containers/Array.h"

#include "TwinLinkCommon.h"

#include "TwinLinkFileDialogLib.h"
#include "TwinLinkPersistentPaths.h"

#include "TwinLinkFloorInfo.h"

#include "TwinLinkCSVExporter.h"
#include "TwinLinkCSVImporter.h"
#include "TwinLinkCSVContents.h"

#include "TwinLinkTickSystem.h"

//// SDK
//#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"

void UTwinLinkFloorInfoSystem::Initialize(FSubsystemCollectionBase& Collection) {
    Collection.InitializeDependency<UTwinLinkTickSystem>();

    FileName = TEXT("FloorInfo.csv");

    ImportFloorInfo();

    // 表示用カテゴリ文字列の設定
    DisplayCategoryMap = {
    {TEXT("Restaurant"), TEXT("飲食店")},
    {TEXT("RetailDealers"), TEXT("物品販売店")},
    {TEXT("ConferenceRoom"), TEXT("会議室")},
    {TEXT("FrontDesk_InformationCenter"), TEXT("フロント・インフォメーションセンター")},
    {TEXT("GuestRoom"), TEXT("客室")},
    {TEXT("FitnessPool"), TEXT("フィットネス・プール")},
    {TEXT("WeddingChubell"), TEXT("ウェディングチャベル")},
    {TEXT("Other"), TEXT("その他")},
    };

    // 表示用カテゴリ文字列を管理用カテゴリに変換するための設定
    for (const auto& Value : DisplayCategoryMap) {
        CategoryMap.Add(Value.Value, Value.Key);
    }
}

void UTwinLinkFloorInfoSystem::Deinitialize() {
}

void UTwinLinkFloorInfoSystem::SelectedFloor(const FString& InFeatureID, const FString& InGrpID, const FString& InKey) {
    SelectedFeatureID = InFeatureID;
    SelectedGrpID = InGrpID;
    SelectedKey = InKey;
}

FString UTwinLinkFloorInfoSystem::GetKeyBySelectedFloor() {
    return SelectedFeatureID + SelectedKey;
}

FString UTwinLinkFloorInfoSystem::GetGrpIDBySelectedFloor() {
    return SelectedGrpID;
}

bool UTwinLinkFloorInfoSystem::CheckAddableFloorInfo(
    const FString& InName,
    const FString& InCategory,
    const FVector& InLocation,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {

    const auto bIsValid = CheckValidFloorInfo(
        InName,
        InCategory,
        InLocation,
        InImageFileName,
        InGuideText,
        InOpningHoursText);

    if (bIsValid == false) {
        return false;
    }

    return true;
}

bool UTwinLinkFloorInfoSystem::CheckValidFloorInfo(
    const FString& InName,
    const FString& InCategory,
    const FVector& InLocation,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {
    return UTwinLinkFloorInfo::IsValid(InName, InCategory, InLocation, InImageFileName, InGuideText, InOpningHoursText);
}

void UTwinLinkFloorInfoSystem::AddFloorInfo(
    const FString& Key,
    const FString& InName,
    const FString& InCategory,
    const FVector& InLocation,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {

    // 登録済みのカテゴリかチェック
    check(DisplayCategoryMap.Find(InCategory) != nullptr);

    // 整合性チェック
    check(UTwinLinkFloorInfo::IsValid(
        InName,
        InCategory,
        InLocation,
        InImageFileName,
        InGuideText,
        InOpningHoursText));

    // データ作成
    UTwinLinkFloorInfo* Info = NewObject<UTwinLinkFloorInfo>();
    Info->Setup(
        InName,
        InCategory,
        InLocation,
        InImageFileName,
        InGuideText,
        InOpningHoursText);

    // 存在しなければ対応するコレクションを作成、あればそのコレクションを使用する
    TObjectPtr<UTwinLinkFloorInfoCollection>* CollectionPtrPtr = FloorInfoCollectionMap.Find(Key);
    TObjectPtr<UTwinLinkFloorInfoCollection> CollectionPtr = nullptr;
    if (CollectionPtrPtr == nullptr) {
        CollectionPtr = FloorInfoCollectionMap.Add(
            Key, NewObject<UTwinLinkFloorInfoCollection>());
    }
    else {
        CollectionPtr = *CollectionPtrPtr;
    }

    // 情報を追加する
    CollectionPtr->Add(Info);

    // 情報を外部ファイルに出力する
    ExportFloorInfo();
}

void UTwinLinkFloorInfoSystem::RemoveFloorInfo(const FString& Key, const TWeakObjectPtr<UTwinLinkFloorInfo>& FloorInfo) {
    check(Key.IsEmpty() == false);
    check(FloorInfo.IsValid());

    const auto CollectionPtrPtr = FloorInfoCollectionMap.Find(Key);
    check(CollectionPtrPtr != nullptr);;
    (*CollectionPtrPtr)->Remove(FloorInfo);
}

void UTwinLinkFloorInfoSystem::ExportFloorInfo() {
    TwinLinkCSVExporter CSVExporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    CSVExporter.SetHeaderContents(
        CSVContents.CreateHeaderContents(
            TEXT("name, category, pos_x, pos_y, pos_z, imageFileName, description, spot_info")));

    FString StringBuf;

    for (const auto& FloorInfoPair : FloorInfoCollectionMap) {
        StringBuf.Reset();
        const auto& FolderName = FloorInfoPair.Key;
        const auto& FloorInfoCollection = FloorInfoPair.Value;

        if (FloorInfoCollection == nullptr)
            continue;

        // ファイルの書き込み
        for (const auto& FloorInfo : *FloorInfoCollection) {
            const auto& Val = FloorInfo.Value;
            const auto Name = Val->GetName();
            const auto Category = Val->GetCategory();
            const auto Location = Val->GetLocation();
            const auto ImageFileName = Val->GetImageFileName();
            const auto GuideText = Val->GetGuideText();
            const auto OpningHoursText = Val->GetOpningHoursText();
            StringBuf =
                CSVContents.CreateBodyContents(
                    FString::Printf(TEXT("%s,%s,%f,%f,%f,%s,%s,%s"),
                        *Name, *Category,
                        Location.X, Location.Y, Location.Z,
                        *ImageFileName,
                        *GuideText, *OpningHoursText));

            CSVExporter.AddBodyContents(StringBuf);
        }

        // コンテンツが追加されているなら
        if (true) {
            // フォルダーの作成
            const auto FolderPath = TwinLinkPersistentPaths::CreateFloorInfoFolderPath(FolderName);
            UTwinLinkFileDialogLib::CreateDirectoryTree(FolderPath);

            // エキスポート
            const auto Filepath = TwinLinkPersistentPaths::CreateFloorInfoFilePath(FolderName, FileName);
            const auto bIsSuc = CSVExporter.ExportCSV(*Filepath);
            check(bIsSuc);
            // 所持コンテンツのリセット
            CSVExporter.ResetBodyContents();
        }
    }

}

void UTwinLinkFloorInfoSystem::ImportFloorInfo() {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("Import floor info"));

    TwinLinkCSVImporter CSVImporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);


    // データが保存されているフォルダの起点フォルダへのパスを作成する
    const auto RootDirectory = TwinLinkPersistentPaths::CreateFloorInfoFolderPath();
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("Root Dir : %s"), *RootDirectory);

    // 起点フォルダー直下のフォルダーを取得する
    TArray<FString> DirectoryPathCollection;
    auto& FileManager = FPlatformFileManager::Get();
    auto& PlatformFile = FileManager.GetPlatformFile();
    bool isSuc = PlatformFile.IterateDirectory(
        *RootDirectory,
        [&DirectoryPathCollection](const TCHAR* FilenameOrDirectory, bool bIsDirectory) {
            if (bIsDirectory == false)
                return true;

            DirectoryPathCollection.Add(FilenameOrDirectory);
            return true; 
        });

    check(isSuc);

    // 既存のデータをリセット
    FloorInfoCollectionMap.Reset();

    // フォルダー単位で読み込み
    for (const auto& DirectoryPath : DirectoryPathCollection) {
        const auto Filepath = FString::Printf(TEXT("%s/%s"), *DirectoryPath, *FileName);
        const auto bIsSuc = CSVImporter.ImportCSV(*Filepath, &CSVContents);

        if (bIsSuc == false) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Import CSV : %s"), *Filepath);
            check(false);
            return;
        }

        // データを解析して取得する
        const auto& BodyContents = CSVContents.GetBodyContents();
        for (const auto& BodyContentsElement : BodyContents) {
            for (const auto& StrDataElement : BodyContentsElement) {
                UE_TWINLINK_LOG(LogTemp, Log, TEXT("Log Contains: %s"), *StrDataElement);
            }

            if (BodyContentsElement.IsEmpty())
                continue;

            // データを解析する
            const auto FloorInfo = NewObject<UTwinLinkFloorInfo>();
            FloorInfo->Setup(BodyContentsElement);

            // 無効なデータか
            if (FloorInfo->GetName().IsEmpty()) {
                // エラーデータを出力する デバッグ用
                for (const auto& ErrorStrDataElement : BodyContentsElement) {
                    UE_TWINLINK_LOG(LogTemp, Log, TEXT("Contains invalid data : %s"), *ErrorStrDataElement);
                }
                // このデータの追加をスキップする
                continue;
            }

            const auto DirectoryName = FPaths::GetPathLeaf(DirectoryPath);
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Directory Name : %s"), *DirectoryName);

            // データを追加する
            TObjectPtr<UTwinLinkFloorInfoCollection> FloorInfoCollection =
                FloorInfoCollectionMap.FindOrAdd(
                    DirectoryName, NewObject<UTwinLinkFloorInfoCollection>());

            FloorInfoCollection->Add(FloorInfo);
        }
    }
}

FString UTwinLinkFloorInfoSystem::ConvertSystemCategoryToDisplayCategoryName(FString Category) {
    const auto Ret = DisplayCategoryMap.Find(Category);
    if (Ret == nullptr) {
        return TEXT("");
    }
    return *Ret;
}

FString UTwinLinkFloorInfoSystem::ConvertDisplayCategoryNameToSystemCategory(FString DisplayCategory) {
    const auto Ret = CategoryMap.Find(DisplayCategory);
    if (Ret == nullptr) {
        return TEXT("");
    }
    return *Ret;
}

TArray<FString> UTwinLinkFloorInfoSystem::GetCategoryDisplayNameCollection() {
    TArray<FString> ValueArray;
    CategoryMap.GenerateKeyArray(ValueArray);
    return ValueArray;
}

bool UTwinLinkFloorInfoSystem::EditFloorInfo(
    const TWeakObjectPtr<UTwinLinkFloorInfo>& FloorInfo, const FString& Name, 
    const FString& Category, const FString& ImageFileName, 
    const FString& Guide, const FString& SpotInfo) {
    check(FloorInfo.IsValid());
    const auto Location = FloorInfo->GetLocation();
    FloorInfo->Setup(Name, Category, Location, ImageFileName, Guide, SpotInfo);
    return false;
}

FString UTwinLinkFloorInfoSystem::CreateFloorInfoMapKey(const FString& FeatureID, const FString& Floor) {
    return FeatureID + Floor;
}

TWeakObjectPtr<UTwinLinkObservableCollection> UTwinLinkFloorInfoSystem::GetFloorInfoCollection(const FString& Key) {
    TObjectPtr<UTwinLinkFloorInfoCollection>* Collection = FloorInfoCollectionMap.Find(Key);

    UTwinLinkFloorInfoCollection* Ret = nullptr;
    if (Collection == nullptr) {
        Ret = FloorInfoCollectionMap.Add(Key, NewObject<UTwinLinkFloorInfoCollection>());
    }
    else {
        Ret = Collection->Get();
    }

    return Ret;
}
