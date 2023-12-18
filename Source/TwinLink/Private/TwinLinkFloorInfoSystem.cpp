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

namespace {
    inline const FString CommonInfoFileName = TEXT("CommonFloorInfo.ini");
}

void UTwinLinkFloorInfoSystem::Initialize(FSubsystemCollectionBase& Collection) {
    Collection.InitializeDependency<UTwinLinkTickSystem>();

    BuildingDesingFilePath =
        TwinLinkPersistentPaths::CreateBuildingDesignFilePath(TEXT("BuildingDesign.csv"));

    ImportBuildingDesign();

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

void UTwinLinkFloorInfoSystem::SelectedFloor(const FString& InFeatureID, const FString& InGrpID, const FString& InKey, UStaticMeshComponent* FloorComponent) {
    SelectedFeatureID = InFeatureID;
    SelectedGrpID = InGrpID;
    SelectedKey = InKey;
    SelectedFloorComponent = FloorComponent;
}

FString UTwinLinkFloorInfoSystem::GetKeyBySelectedFloor() {
    return SelectedFeatureID + SelectedKey;
}

FString UTwinLinkFloorInfoSystem::GetGrpIDBySelectedFloor() {
    return SelectedGrpID;
}

UStaticMeshComponent* UTwinLinkFloorInfoSystem::GetSelectedFloorComponent() const {
    return SelectedFloorComponent;
}

UTwinLinkFloorInfo* UTwinLinkFloorInfoSystem::GetInfoByUV(const FVector2D& UV) const {
    for (const auto& [_, Collection] : FloorInfoCollectionMap) {
        for (const auto& [Key, Info] : *Collection) {
            if (Info->GetUV() == UV)
                return Info;
        }
    }

    return nullptr;
}

bool UTwinLinkFloorInfoSystem::CheckAddableFloorInfo(
    const FString& InName,
    const FString& InCategory,
    const FVector& InLocation,
    const FVector2D& InUV,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {

    const auto bIsValid = CheckValidFloorInfo(
        InName,
        InCategory,
        InLocation,
        InUV,
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
    const FVector2D& InUV,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {
    return UTwinLinkFloorInfo::IsValid(InName, InCategory, InLocation, InUV, InImageFileName, InGuideText, InOpningHoursText);
}

void UTwinLinkFloorInfoSystem::AddFloorInfo(
    const FString& Key,
    const FString& InName,
    const FString& InCategory,
    const FVector& InLocation,
    const FVector2D& InUV,
    const FString& InImageFileName,
    const FString& InGuideText,
    const FString& InOpningHoursText) {

    // 登録済みのカテゴリかチェック
    if (DisplayCategoryMap.Find(InCategory) == nullptr ||
        !UTwinLinkFloorInfo::IsValid(
            InName,
            InCategory,
            InLocation,
            InUV,
            InImageFileName,
            InGuideText,
            InOpningHoursText)) {
        UE_TWINLINK_LOG(LogTemp, Error, TEXT("Invalid data element in floow info"));
        return;
    }

    // データ作成
    UTwinLinkFloorInfo* Info = NewObject<UTwinLinkFloorInfo>();
    Info->Setup(
        InName,
        InCategory,
        InLocation,
        InUV,
        InImageFileName,
        InGuideText,
        InOpningHoursText);

    // 存在しなければ対応するコレクションを作成、あればそのコレクションを使用する
    TObjectPtr<UTwinLinkFloorInfoCollection>* CollectionPtrPtr = FloorInfoCollectionMap.Find(Key);
    TObjectPtr<UTwinLinkFloorInfoCollection> CollectionPtr;
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
    // 共通情報をエクスポート
    ExportCommonInfo();

    TwinLinkCSVExporter CSVExporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    CSVExporter.SetHeaderContents(
        CSVContents.CreateHeaderContents(
            TEXT("name, category, pos_x, pos_y, pos_z, uv_x, uv_y, imageFileName, description, spot_info")));

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
            const auto UV = Val->GetUV();
            const auto ImageFileName = Val->GetImageFileName();
            const auto GuideText = Val->GetGuideText();
            const auto OpningHoursText = Val->GetOpningHoursText();
            StringBuf =
                CSVContents.CreateBodyContents(
                    TArray<FString> {
                *Name, * Category,
                    FString::SanitizeFloat(Location.X), FString::SanitizeFloat(Location.Y), FString::SanitizeFloat(Location.Z),
                    FString::SanitizeFloat(UV.X), FString::SanitizeFloat(UV.Y),
                    * ImageFileName,
                    * GuideText, * OpningHoursText});

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

    // 共通情報をインポート
    ImportCommonInfo();

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

TWeakObjectPtr<UTwinLinkBuildingDesignInfo> UTwinLinkFloorInfoSystem::FindBuildingDesign(const FString& Key) {
    TObjectPtr<UTwinLinkBuildingDesignInfo>* DataPtr = BuidingDesingInfoMap.Find(Key);
    if (DataPtr == nullptr)
        return nullptr;
    check(*DataPtr);
    return *DataPtr;
}

void UTwinLinkFloorInfoSystem::AddBuildingDesign(const FString& Key, const FString& ImageFileName) {
    const auto bIsContains = BuidingDesingInfoMap.Contains(Key);
    check(bIsContains == false);    // 既に存在するキーで追加しようとした

    TObjectPtr<UTwinLinkBuildingDesignInfo> Data = NewObject<UTwinLinkBuildingDesignInfo>();
    Data->Setup(ImageFileName);

    BuidingDesingInfoMap.Add(Key, Data);

    if (EvOnAddedBuildingDesignInfoInstance.IsBound()) {
        EvOnAddedBuildingDesignInfoInstance.Broadcast();
    }

    ExportBuildingDesign();
}

bool UTwinLinkFloorInfoSystem::EditBuildingDesign(const FString& Key, const TWeakObjectPtr<UTwinLinkBuildingDesignInfo>& BuidlingDesignInfo, const FString& ImageFileName) {
    TObjectPtr<UTwinLinkBuildingDesignInfo>* DataPtr = BuidingDesingInfoMap.Find(Key);
    if (DataPtr == nullptr) // 存在しないデータにアクセス使用とした
        return false;
    if (*DataPtr != BuidlingDesignInfo.Get())   // キーとデータが紐づいていない
        return false;

    (*DataPtr)->Setup(ImageFileName);

    ExportBuildingDesign();
    return true;
}

bool UTwinLinkFloorInfoSystem::RemoveBuildingDesign(const FString& Key, const TWeakObjectPtr<UTwinLinkBuildingDesignInfo> BuidlingDesignInfoPtr) {
    if (BuidlingDesignInfoPtr == nullptr)
        return false;

    TObjectPtr<UTwinLinkBuildingDesignInfo>* DataPtr = BuidingDesingInfoMap.Find(Key);
    if (DataPtr == nullptr) // 存在しないデータにアクセス使用とした
        return false;
    if (*DataPtr != BuidlingDesignInfoPtr)   // キーとデータが紐づいていない
        return false;

    BuidlingDesignInfoPtr->EvOnDeleted.Broadcast();
    BuidingDesingInfoMap.Remove(Key);

    ExportBuildingDesign();

    return true;
}

void UTwinLinkFloorInfoSystem::ExportBuildingDesign() {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink Export BuildingDesignInfo : %s"), *BuildingDesingFilePath);

    TwinLinkCSVExporter CSVExporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    CSVExporter.SetHeaderContents(
        CSVContents.CreateHeaderContents(
            TEXT("key, image_file_name")));

    FString StringBuf;
    for (const auto& BuildingDesingInfo : BuidingDesingInfoMap) {
        const auto& Key = BuildingDesingInfo.Key;
        const auto& Val = BuildingDesingInfo.Value;
        const auto ImageFileName = Val->GetImageFileName();
        StringBuf =
            CSVContents.CreateBodyContents(
                FString::Printf(TEXT("%s,%s"),
                    *Key,
                    *ImageFileName));

        CSVExporter.AddBodyContents(StringBuf);
    }

    const auto bIsSuc = CSVExporter.ExportCSV(*BuildingDesingFilePath);
    check(bIsSuc);
}

void UTwinLinkFloorInfoSystem::ImportBuildingDesign() {
    TwinLinkCSVImporter CSVImporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    const auto bIsSuc = CSVImporter.ImportCSV(*BuildingDesingFilePath, &CSVContents);

    if (bIsSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Import CSV : %s"), *BuildingDesingFilePath);
        return;
    }

    // データを解析して取得する
    const auto& BodyContents = CSVContents.GetBodyContents();
    BuidingDesingInfoMap.Reset();
    for (const auto& BodyContentsElement : BodyContents) {
        for (const auto& ErrorStrDataElement : BodyContentsElement) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Log Contains: %s"), *ErrorStrDataElement);
        }

        const auto Key = BodyContentsElement[0];
        const auto Val = BodyContentsElement[1];

        // データを解析する
        const auto BuildingDesignInfo = NewObject<UTwinLinkBuildingDesignInfo>();
        BuildingDesignInfo->Setup(Val);

        // 無効なデータか
        if (BuildingDesignInfo->IsValid() == false) {
            // エラーデータを出力する デバッグ用
            for (const auto& ErrorStrDataElement : BodyContentsElement) {
                UE_TWINLINK_LOG(LogTemp, Log, TEXT("Contains invalid data : %s"), *ErrorStrDataElement);
            }
            // このデータの追加をスキップする
            continue;
        }

        // データを追加する
        BuidingDesingInfoMap.Add(Key, BuildingDesignInfo);
    }
}

bool UTwinLinkFloorInfoSystem::EditFloorInfo(
    const TWeakObjectPtr<UTwinLinkFloorInfo>& FloorInfo, const FString& Name,
    const FString& Category, const FString& ImageFileName,
    const FString& Guide, const FString& SpotInfo) {
    check(FloorInfo.IsValid());
    const auto Location = FloorInfo->GetLocation();
    const auto UV = FloorInfo->GetUV();
    FloorInfo->Setup(Name, Category, Location, UV, ImageFileName, Guide, SpotInfo);
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

TWeakObjectPtr<UTwinLinkFloorInfoCollection> UTwinLinkFloorInfoSystem::GetFloorInfoCollectionOrDefault(
    const FString& Key) const {
    const auto Ret = FloorInfoCollectionMap.Find(Key);
    if (!Ret)
        return nullptr;

    return *Ret;
}

bool UTwinLinkFloorInfoSystem::IsFloorVisible(const FString& Key) const {
    // デフォがtrueなのでキーが存在しない時もtrue
    const auto Info = FloorInfoCollectionMap.Find(Key);
    return !Info || Info->Get()->IsFloorVisible();
}

void UTwinLinkFloorInfoSystem::SetFloorVisible(const FString& Key, bool Visible) {
    if (const auto Info = Cast<UTwinLinkFloorInfoCollection>(GetFloorInfoCollection(Key)))
        Info->SetFloorVisible(Visible);
}

bool UTwinLinkFloorInfoSystem::ExportCommonInfo() {
    const auto CommonInfoFilePath = TwinLinkPersistentPaths::CreateFloorInfoFolderPath(*CommonInfoFileName);
    // フォルダーの作成
    const auto FolderPath = TwinLinkPersistentPaths::CreateFloorInfoFolderPath();
    UTwinLinkFileDialogLib::CreateDirectoryTree(FolderPath);

    // エキスポート
    TArray<FString> HideFloorKeys;
    for (auto& F : FloorInfoCollectionMap) {
        if (F.Value->IsFloorVisible())
            continue;
        HideFloorKeys.Add(F.Key);
    }

    TArray<FString> Lines;
    // 非表示階層の出力
    {
        const auto FloorKeys = FString::Join(HideFloorKeys, TEXT(","));
        Lines.Add(FString::Printf(TEXT("HideFloor:%s"), *FloorKeys));
    }
    return FFileHelper::SaveStringArrayToFile(Lines, *CommonInfoFilePath);
}

bool UTwinLinkFloorInfoSystem::ImportCommonInfo() {
    const auto CommonInfoFilePath = TwinLinkPersistentPaths::CreateFloorInfoFolderPath(*CommonInfoFileName);
    //ファイル取得
    FString TextData;
    const auto bIsSucLoad = FFileHelper::LoadFileToString(TextData, *CommonInfoFilePath);
    // ファイルの読み込みに失敗
    if (bIsSucLoad == false) {
        return false;
    }

    //列で分解
    TArray<FString> RowElements;
    TextData.ParseIntoArray(RowElements, TEXT("\n"), true);

    // 
    for (auto& Row : RowElements) {
        Row.TrimStartAndEndInline();
        FString Key;
        FString Value;
        const auto bFound = Row.Split(TEXT(":"), &Key, &Value, ESearchCase::CaseSensitive, ESearchDir::FromStart);
        if (!bFound)
            continue;

        // 非表示設定されているフロア情報
        // HideFloor : [AAA, BBB, CCC, DDD]
        if (Key == TEXT("HideFloor")) {
            TArray<FString> Ids;
            Value.ParseIntoArray(Ids, TEXT(","));

            for (auto& Id : Ids) {
                const auto& FloorInfoCollection =
                    FloorInfoCollectionMap.FindOrAdd(
                        Id, NewObject<UTwinLinkFloorInfoCollection>());
                FloorInfoCollection->SetFloorVisible(false);
            }
        }
    }

    return true;
}
