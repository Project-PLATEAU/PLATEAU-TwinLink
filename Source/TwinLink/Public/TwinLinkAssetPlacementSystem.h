// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "TwinLinkSystemVersionInfo.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkAssetPlacementInfoCollection.h"
#include "TwinLinkAssetPlacementPresetData.h"
#include "TwinLinkAssetPlacementSystem.generated.h"

/**
 * 操作モード
 */
UENUM(BlueprintType)
enum class ETwinLinkAssetPlacementModes : uint8 {
    AssetPlacement,
    AssetTransform,
};

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
     * @param AssetPlacementInfo
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
     * @brief アセットの配置情報を削除する
     * @param TargetActor
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementRemoveActor(AActor* TargetActor);

    /**
     * @brief プリセットアセットを再配置
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementRelocation();

    /**
     * @brief プリセットアセットを登録
     * @param Presets
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementRegistPresetAsset(const FTwinLinkAssetPlacementPresetData& Presets);

    /**
     * @brief 操作モードを取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        ETwinLinkAssetPlacementModes TwinLinkAssetPlacementGetMode() const { return CurrentMode; }
    /**
     * @brief 操作モードを設定
     * @param Mode
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementSetMode(const ETwinLinkAssetPlacementModes Mode) { CurrentMode = Mode; }

    /**
     * @brief 配置未確定アクタを生成
     * @param AssetPlacementInfo
    */
    void SpawnUnsettledActor(const TObjectPtr<UTwinLinkAssetPlacementInfo> AssetPlacementInfo);

    /**
     * @brief 配置未確定アクタの位置を更新
     * @param Location
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementUpdateUnsettledActor(const FVector& Location);

    /**
     * @brief マウスカーソルがWidgetに乗っているか
     * @return
    */
    bool IsWidgetUnderMouseCursor();

    /**
     * @brief 配置未確定アクタの位置を確定
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkAssetPlacementSettledActor();

    /**
     * @brief 配置未確定アクタを取得
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        AActor* TwinLinkAssetPlacementGetUnsettledActor() const { return AssetPlacementUnsettledActor; }
private:

    /**
     * @brief アセットから形状を取得
     * @param PresetID
     * @return
    */
    TObjectPtr<UStaticMesh> GetPresetAssetMesh(const int PresetID);

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

    /** 操作モード **/
    UPROPERTY()
        ETwinLinkAssetPlacementModes CurrentMode = ETwinLinkAssetPlacementModes::AssetTransform;

    /** 配置未確定アクタへの参照 **/
    UPROPERTY()
        TObjectPtr<AActor> AssetPlacementUnsettledActor;

    /** 配置未確定アセットの配置情 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkAssetPlacementInfo> AssetPlacementUnsettledAssetPlacementInfo;
public:
    /** プリセットアセットのIDリスト **/
    UPROPERTY()
        TArray<int> PresetIDs;

    /** プリセットアセットのスタティックメッシュ群 **/
    UPROPERTY()
        TMap<uint32, TObjectPtr<UStaticMesh>> PresetMeshes;

    /** プリセットアセットのウィジェット用テクスチャ群 **/
    UPROPERTY()
        TMap<uint32, TObjectPtr<UTexture2D>> PresetTextures;
};
