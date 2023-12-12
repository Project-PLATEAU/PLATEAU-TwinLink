// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkFacilityInfoSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "TwinLink.h"
#include "TwinLinkCommon.h"

#include "TwinLinkTickSystem.h"

#include "TwinLinkPersistentPaths.h"
#include "TwinLinkFacilityInfo.h"

#include "TwinLinkCSVExporter.h"
#include "TwinLinkCSVImporter.h"
#include "TwinLinkCSVContents.h"

#include "TwinLinkWorldViewer.h"

// SDK
#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"

void UTwinLinkFacilityInfoSystem::Initialize(FSubsystemCollectionBase& Collection) {
    Collection.InitializeDependency<UTwinLinkTickSystem>();

    Filepath = TwinLinkPersistentPaths::CreateFacilityInfoFilePath(TEXT("FacilityInfo.csv"));
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("FacilityInfo File Path : %s"), *Filepath);

    FacilityInfoCollection = NewObject<UTwinLinkFacilityInfoCollection>();
    ImportFacilityInfo();

    // 施設カテゴリ類初期化
    InitializeCategoryMap();
}

void UTwinLinkFacilityInfoSystem::Deinitialize() {
}

void UTwinLinkFacilityInfoSystem::AddFacilityInfo(
    const FString& InName, const FString& InCategory, const FString& InFeatureID, const FString& InImageFileName, const FString& InDescription, const FString& InSpotInfo, std::optional<FVector> Entrance) {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink AddFacilityInfo : %s"), *Filepath);

    // 表示用カテゴリ名を識別用カテゴリ名に変換
    const auto CategoryName = ReverseCategoryDisplayNameMap.Find(InCategory);
    check(CategoryName != nullptr);

    // 施設情報の作成
    TArray<FVector> Entrances;
    if (Entrance.has_value())
        Entrances.Add(*Entrance);
    UTwinLinkFacilityInfo* FacilityInfo = NewObject<UTwinLinkFacilityInfo>();
    const auto bIsSuc = FacilityInfo->Setup(
        InName,
        *CategoryName,
        InFeatureID,
        InImageFileName,
        InDescription,
        InSpotInfo,
        Entrances
    );

    // CheckAddableFacilityInfo()内で チェック済みなので正常な値が格納されている前提で記述する
    check(bIsSuc);

    // 施設情報の追加
    FacilityInfoCollection->Add(FacilityInfo);
}

void UTwinLinkFacilityInfoSystem::RemoveFacilityInfo(const TWeakObjectPtr<UTwinLinkFacilityInfo>& FacilityInfo) {
    check(FacilityInfo.IsValid());
    FacilityInfoCollection->Remove(FacilityInfo);
}

void UTwinLinkFacilityInfoSystem::ExportFacilityInfo() {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink Export FacilityInfo : %s"), *Filepath);

    TwinLinkCSVExporter CSVExporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    CSVExporter.SetHeaderContents(
        CSVContents.CreateHeaderContents(
            TEXT("name, category, featureID, imageFileName, description, spot_info")));

    FString StringBuf;
    for (const auto& FacilityInfo : *FacilityInfoCollection) {
        const auto& Val = FacilityInfo.Value;
        const auto Name = Val->GetName();
        const auto Category = Val->GetCategory();
        const auto FeatureID = Val->GetFeatureID();
        const auto ImageFileName = Val->GetImageFileName();
        const auto Description = Val->GetDescription();
        const auto SpotInfo = Val->GetSpotInfo();

        FString EntranceStr = "";
        if (Val->GetEntrances().IsEmpty() == false) {
            auto& E = Val->GetEntrances()[0];
            EntranceStr = FString::Printf(TEXT("%f#%f#%f"), E.X, E.Y, E.Z);
        }
        StringBuf =
            CSVContents.CreateBodyContents(
                TArray<FString>{
            *(Name),
                * Category, * FeatureID, * ImageFileName,
                * Description, * SpotInfo, * EntranceStr});

        CSVExporter.AddBodyContents(StringBuf);
    }

    const auto bIsSuc = CSVExporter.ExportCSV(*Filepath);
    check(bIsSuc);
}

void UTwinLinkFacilityInfoSystem::ImportFacilityInfo() {
    TwinLinkCSVImporter CSVImporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    const auto bIsSuc = CSVImporter.ImportCSV(*Filepath, &CSVContents);

    if (bIsSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Import CSV : %s"), *Filepath);
        return;
    }

    // データを解析して取得する
    const auto& BodyContents = CSVContents.GetBodyContents();
    FacilityInfoCollection->Reset();
    for (const auto& BodyContentsElement : BodyContents) {
        for (const auto& ErrorStrDataElement : BodyContentsElement) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Log Contains: %s"), *ErrorStrDataElement);
        }

        // データを解析する
        const auto FacilityInfo = NewObject<UTwinLinkFacilityInfo>();
        FacilityInfo->Setup(BodyContentsElement);

        // 無効なデータか
        if (FacilityInfo->GetName() == TEXT("")) {
            // エラーデータを出力する デバッグ用
            for (const auto& ErrorStrDataElement : BodyContentsElement) {
                UE_TWINLINK_LOG(LogTemp, Log, TEXT("Contains invalid data : %s"), *ErrorStrDataElement);
            }
            // このデータの追加をスキップする
            continue;
        }

        // データを追加する
        FacilityInfoCollection->Add(FacilityInfo);
    }
}

bool UTwinLinkFacilityInfoSystem::EditFacilityInfo(
    const TWeakObjectPtr<UTwinLinkFacilityInfo>& FacilityInfo,
    const FString& Name, const FString& Category, const FString& ImageFileName, const FString& Guide, const FString& SpotInfo, std::optional<FVector> Entrance) {
    ensure(FacilityInfo.IsValid());
    if (FacilityInfo.IsValid() == false) {
        return false;
    }

    const auto FeatureID = FacilityInfo->GetFeatureID();

    const auto Facility = FindFacility(FacilityInfo);
    if (Facility.IsValid() == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Can't find Facitliy : %s"), *FacilityInfo->GetFeatureID());
        return false;
    }

    // 表示用カテゴリ名を識別用カテゴリ名に変換
    const auto CategoryName = ReverseCategoryDisplayNameMap.Find(Category);
    check(CategoryName != nullptr);
    TArray<FVector> Entrances;
    if (Entrance.has_value())
        Entrances.Add(*Entrance);
    const auto isSuc = FacilityInfo->Setup(Name, *CategoryName, FeatureID, ImageFileName, Guide, SpotInfo, Entrances);
    if (isSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Setup Facility Info: %s"), *FacilityInfo->GetName());
        return false;
    }

    return true;
}

FString UTwinLinkFacilityInfoSystem::GetCategoryDisplayName(const FString& Category) {
    FString* DisplayName = CategoryDisplayNameMap.Find(Category);
    if (DisplayName == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Not defined facility category:%s"), *Category);
        return TEXT("undefind");
    }
    return *DisplayName;
}

TArray<FString> UTwinLinkFacilityInfoSystem::GetCategoryDisplayNameCollection() {
    TArray<FString> ValueArray;
    CategoryDisplayNameMap.GenerateValueArray(ValueArray);
    return ValueArray;
}

void UTwinLinkFacilityInfoSystem::MoveCharactersNearTheFacility(const TWeakObjectPtr<UPrimitiveComponent>& Facility) {
    check(Facility.IsValid());

    const auto WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
    if (WorldViewer == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("移動対象が存在しない"));
        return;
    }

    // NavigationBoundsとWorldViewerの位置から適切な移動先座標をそれっぽく計算する　
    // NavigationBoundsの表面上からWorldViewerの座標に近い点をA、NavigationBoundsの中心をBとして
    // A + (A - B)　を移動先としている。　表面上の点の座標にNavigationBoundsの中心から遠ざかるようなベクトルを足している

    const auto NavigationBounds = Facility.Get()->GetNavigationBounds();
    const FVector FacilityClosestPoint = NavigationBounds.GetClosestPointTo(WorldViewer->GetActorLocation());
    const FVector FacilityCenter = NavigationBounds.GetCenter();
    const FVector VecCenterToClosestPoint = FacilityClosestPoint - FacilityCenter;
    const float DistanceFactor = 5.0f;  // てきとうな値　数値が大きいほど距離が離れる
    const float HighFactor = 3000.0f;

    // 座標を設定する
    const FVector NewPosition =
        FacilityClosestPoint + VecCenterToClosestPoint * DistanceFactor + FVector::UpVector * HighFactor;

    // 向きを設定する
    FRotator NewRotation =
        UKismetMathLibrary::FindLookAtRotation(
            NewPosition, FacilityCenter);

    WorldViewer->SetLocation(NewPosition, NewRotation, 2.0f);
}

TWeakObjectPtr<UPrimitiveComponent> UTwinLinkFacilityInfoSystem::FindFacility(const TWeakObjectPtr<UTwinLinkFacilityInfo>& FacilityInfo) {
    check(FacilityInfo.IsValid());
    return FindFacility(FacilityInfo->GetFeatureID());
}

TWeakObjectPtr<UPrimitiveComponent> UTwinLinkFacilityInfoSystem::FindFacility(const FString& FeatureId) const {
    /*
     コンポーネントを取得するためにアクター
     tagかクラスで検索する　
     */
    TArray<AActor*> PLATEAUActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APLATEAUInstancedCityModel::StaticClass(), PLATEAUActors);
    if (PLATEAUActors.IsEmpty()) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed find actor"));
        return nullptr;
    }

    for (auto& PLATEAUActor : PLATEAUActors) {
        TArray<UActorComponent*> FeatureComponents;
        PLATEAUActor->GetComponents(FeatureComponents, true);
        if (FeatureComponents.IsEmpty()) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed find components"));
            return nullptr;
        }

        for (auto& FeatureComponent : FeatureComponents) {
            if (FeatureId.Contains(FeatureComponent->GetName())) {
                return Cast<UPrimitiveComponent>(FeatureComponent);
            }
        }
    }

    // 見つからなかった
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("Faild find %s"), *FeatureId);
    return nullptr;
}

TWeakObjectPtr<UTwinLinkObservableCollection> UTwinLinkFacilityInfoSystem::GetFacilityInfoCollection() const {
    return FacilityInfoCollection;
}

const TMap<uint32, TObjectPtr<UTwinLinkFacilityInfo>>& UTwinLinkFacilityInfoSystem::
GetFacilityInfoCollectionAsMap() const {
    return FacilityInfoCollection->GetFacilityInfoCollection();
}

TWeakObjectPtr<UTwinLinkFacilityInfo> UTwinLinkFacilityInfoSystem::FindFacilityInfoByFeatureId(
    const FString& FeatureId) const {
    for (auto& Item : GetFacilityInfoCollectionAsMap()) {
        if (Item.Value->GetFeatureID() == FeatureId)
            return Item.Value.Get();
    }
    return nullptr;
}

TArray<UStaticMeshComponent*> UTwinLinkFacilityInfoSystem::GetStaticMeshComponents(const FString& Key) {
    if (StaticMeshComponentMap.IsEmpty()) {
        const auto CityModel = Cast<APLATEAUInstancedCityModel>(FTwinLinkModule::Get().GetCityModel());
        TArray<UPLATEAUCityObjectGroup*> CityObjects;
        CityModel->GetComponents<UPLATEAUCityObjectGroup>(CityObjects);
        for (const auto& CityObject : CityObjects) {
            FString Name = CityModel->GetCityObjectInfo(CityObject).ID;//CityObject->GetName();
            TArray<UStaticMeshComponent*> Array;
            if (StaticMeshComponentMap.Contains(Name)) {
                StaticMeshComponentMap[Name].Add(Cast<UStaticMeshComponent>(CityObject));
                continue;
            }
            Array.Add(Cast<UStaticMeshComponent>(CityObject));
            StaticMeshComponentMap.Add(Name, Array);
        }
    }

    if (StaticMeshComponentMap.Contains(Key)) {
        return StaticMeshComponentMap[Key];
    }
    return TArray<UStaticMeshComponent*>();
}

void UTwinLinkFacilityInfoSystem::InitializeCategoryMap() {
    // 必要に応じて外部ファイルを読み込むようにする

    CategoryDisplayNameMap = {
    {TEXT("Shopping"), TEXT("商業施設")},
    {TEXT("Transportation"), TEXT("交通施設")},
    {TEXT("Tourist"), TEXT("観光地")},
    {TEXT("Food"), TEXT("飲食店")},
    };

    // 逆検索用のマップの作成
    for (const auto& Value : CategoryDisplayNameMap) {
        ReverseCategoryDisplayNameMap.Add(Value.Value, Value.Key);
    }
}