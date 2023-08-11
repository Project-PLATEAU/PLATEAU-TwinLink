// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "TwinLinkSystemVersionInfo.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkAssetPlacementInfoCollection.h"
#include "TwinLinkAssetPlacementSystem.generated.h"

/**
 * アセット配置情報の管理を行うサブシステム
 */
UCLASS()
class TWINLINK_API UTwinLinkAssetPlacementSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()

public:

    /**
     * @brief
     * @param Collection
    */
    virtual void Initialize(FSubsystemCollectionBase& Collection);

    /**
     * @brief
    */
    virtual void Deinitialize();

public:

    /**
     * @brief アセット配置情報群をエキスポートする
    */
    void ExportAssetPlacementInfo();

    /**
     * @brief アセット配置情報群をインポートする
    */
    void ImportAssetPlacementInfo();

    /**
     * @brief アセット配置情報群を保持する通知機能付きのコレクションを取得する
     * @return
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> GetAssetPlacementInfoCollection() const;

    /**
     * @brief アセットの配置を行う
     * @return
    */
    void AddAssetPlacementActor(const TObjectPtr<UTwinLinkAssetPlacementInfo> AssetPlacementInfo);

    /**
     * @brief アセットの配置情報を更新する
     * @param TargetActor
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementUpdateActor(AActor* TargetActor);

    /**
     * @brief アセットのアクタを生成する
     * @param Soil
     * @param Location
     * @param Rotation
     * @return
    */
    AActor* SpawnAssetPlacementActor(const TObjectPtr<UStaticMesh> Soil, const FVector& Location, const FRotator& Rotation);

    /**
     * @brief アセット配置テスト用の形状を登録
     * @param Soil
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementRegistSoilTest(UStaticMesh* Soil);

private:

    /**
     * @brief アセットから形状を取得
     * @param PresetID
     * @return
    */
    TObjectPtr<UStaticMesh> GetAsset(const int PresetID);

private:

    /** アセット配置情報機能のバージョン　永続化時に組み込む **/
    const TwinLinkSystemVersionInfo VersionInfo = TwinLinkSystemVersionInfo(0, 0, 0);

    /** アセット配置情報の永続化時のパス **/
    FString Filepath;

    /** アセット配置情報群 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkAssetPlacementInfoCollection> AssetPlacementInfoCollection;

    /** 配置したアクタへの参照 **/
    UPROPERTY()
        TMap<uint32, TObjectPtr<AActor>> AssetPlacementActorCollection;

    /** テスト用アセットへの参照 **/
    TObjectPtr<UStaticMesh> TestAssetSoil;
};
