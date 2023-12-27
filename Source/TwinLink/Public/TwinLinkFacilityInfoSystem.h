// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"

#include "TwinLinkSystemVersionInfo.h"
#include "TwinLinkFacilityInfoCollection.h"

#include "TwinLinkFacilityInfoSystem.generated.h"

// 視点情報
class UTwinLinkFacilityInfo;

// 視点操作機能を提供するキャラクター
class ATwinLinkWorldViewer;

// 地物のインスタンス
class UPrimitiveComponent;

/**
 * 施設情報を管理、利用するシステム
 */
UCLASS()
class TWINLINK_API UTwinLinkFacilityInfoSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()

public:

    /**
     * @brief 初期化
     * @param Collection
    */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /**
     * @brief 解放処理
    */
    virtual void Deinitialize() override;

public:
    /**
     * @brief 施設情報を追加する
     * @param _Name
    */
    void AddFacilityInfo(
        const FString& InName, const FString& InCategory,
        const FString& InFeatureID, const FString& InImageFileName,
        const FString& InDescription, const FString& InSpotInfo, std::optional<FVector> Entrance);

    /**
     * @brief 施設情報を削除する
     * @param FacilityInfo
    */
    void RemoveFacilityInfo(const TWeakObjectPtr<UTwinLinkFacilityInfo>& FacilityInfo);

    /**
     * @brief 施設情報群をエキスポートする
    */
    void ExportFacilityInfo();

    /**
     * @brief 施設情報群をインポートする
    */
    void ImportFacilityInfo();

    /**
     * @brief 施設情報の編集
     * @param FacilityInfo
     * @param Name
     * @param Category
     * @param ImageFileName
     * @param Guide
     * @param SpotInfo
     * @param Entrance
     * @return 成否
    */
    bool EditFacilityInfo(
        const TWeakObjectPtr<UTwinLinkFacilityInfo>& FacilityInfo,
        const FString& Name,
        const FString& Category,
        const FString& ImageFileName,
        const FString& Guide,
        const FString& SpotInfo,
        std::optional<FVector> Entrance
    );

    /**
     * @brief カテゴリの表示名を取得する
     * @param Category
     * @return 表示名
    */
    FString GetCategoryDisplayName(const FString& Category);

    /**
     * @brief カテゴリの表示名の一覧を取得する
     * @return 表示名一覧
    */
    TArray<FString> GetCategoryDisplayNameCollection();

    /**
     * @brief 施設近くにキャラクターを移動させる
    */
    void MoveCharactersNearTheFacility(const TWeakObjectPtr<UPrimitiveComponent>& Facility);

    /**
     * @brief 地物を施設情報から検索する
     * @param FacilityInfo
     * @return
    */
    TWeakObjectPtr<UPrimitiveComponent> FindFacility(const TWeakObjectPtr<UTwinLinkFacilityInfo>& FacilityInfo);

    /**
     * @brief 地物を建物Idから検索する
     * @param FeatureId
     * @return
    */
    TWeakObjectPtr<UPrimitiveComponent> FindFacility(const FString& FeatureId) const;

    /**
     * @brief 施設情報群を保持する通知機能付きのコレクションを取得する
     * @return
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> GetFacilityInfoCollection() const;

    /*
     * @brief : 施設情報群をマップとして取得する
     */
    const TMap<uint32, TObjectPtr<UTwinLinkFacilityInfo>>& GetFacilityInfoCollectionAsMap() const;

    /*
     * @brief : 地物IDからUTwinLinkFacilityInfoを取得する. 全探索するので重い
     */
    TWeakObjectPtr<UTwinLinkFacilityInfo> FindFacilityInfoByFeatureId(const FString& FeatureId) const;

    /*
     * @brief : 地物IDからスタティックメッシュコンポーネントを取得
     */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        TArray<UStaticMeshComponent*> GetStaticMeshComponents(const FString& Key);

private:
    /** 施設情報機能のバージョン　永続化時に組み込む **/
    const TwinLinkSystemVersionInfo VersionInfo = TwinLinkSystemVersionInfo(1, 0, 0);

    /** 施設情報の永続化時のパス **/
    FString Filepath;

    /** 施設情報群 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkFacilityInfoCollection> FacilityInfoCollection;

    /** カテゴリ名をキー、カテゴリ表示名を値としたマップ **/
    UPROPERTY()
        TMap<FString, FString> CategoryDisplayNameMap;
    UPROPERTY()
        TMap<FString, FString> ReverseCategoryDisplayNameMap;
private:
    /**
     * @brief カテゴリ名変換用のマップを初期化する
    */
    void InitializeCategoryMap();

    /** 地物IDに対応したスタティックメッシュコンポーネント **/
    TMap<FString, TArray<UStaticMeshComponent*>> StaticMeshComponentMap;
};
