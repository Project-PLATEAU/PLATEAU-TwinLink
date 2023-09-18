// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"
#include "TwinLinkNavSystemDef.h"
#include "TwinLinkNavSystemFindPathInfo.h"
#include "TwinLinkNavSystemPathDrawer.h"
#include "TwinLinkNavSystemParam.h"
#include "TwinLinkNavSystemFindPathUiInfo.h"
#include "TwinLinkNavSystem.generated.h"
class ATwinLinkNavSystemPathFinder;
class APLATEAUInstancedCityModel;
class ANavMeshBoundsVolume;

/*
 * @brief : パス検索などランタイム中のナビメッシュ関係を管理するクラス.
 */
UCLASS()
class TWINLINK_API ATwinLinkNavSystem : public AActor {
    GENERATED_BODY()
public:
    ATwinLinkNavSystem();

    virtual void Tick(float DeltaSeconds) override;

    ECollisionChannel GetDemCollisionChannel() const {
        return DemCollisionChannel;
    }

    FBox GetDemCollisionAabb() const {
        return DemCollisionAabb;
    }

    void SetDemCollisionAabb(const FBox& val) {
        DemCollisionAabb = val;
    }

    /*
     * @brief : 検索されたパスの高さチェックの間隔
     */
    float GetPathFindingHeightCheckInterval() const {
        return RuntimeParam->PathPointInterval;
    }

    /*
     * @brief : モード切替
     */
    UFUNCTION(BlueprintCallable)
        void ChangeMode(NavSystemMode Mode, bool bForce = false);

    /*
     * @brief : パス検索の結果を返す. nullptrの場合は検索していないか検索途中
     */
    UFUNCTION(BlueprintCallable)
        bool TryGetReadyFindPathInfo(FTwinLinkNavSystemFindPathInfo& Out) const;

    /*
     * @brief : パス検索可能になったときに呼ばれるイベント
     */
    UFUNCTION(BlueprintImplementableEvent)
        void OnReadyFindPathInfo(const FTwinLinkNavSystemFindPathInfo& PathInfo) const;

    /*
     * @brief : ランタイムで使うパラメータ取得
     */
    UFUNCTION(BlueprintCallable)
        const UTwinLinkNavSystemParam* GetRuntimeParam() const;

    /*
     * @brief : 移動パスの情報取得
     */
    UFUNCTION(BlueprintCallable)
        FTwinLinkNavSystemFindPathUiInfo GetDrawMoveTimeUiInfo(const FBox2D& ScreenRange) const;

    /*
     * @brief : 現在のパス検索モードアクタ
     */
    UFUNCTION(BlueprintCallable)
        ATwinLinkNavSystemPathFinder* GetNowPathFinder();

    UFUNCTION(BlueprintCallable)
    const FTwinLinkNavSystemBuildingInfo& GetBaseBuilding() const {
        return BaseBuilding;
    }

    UFUNCTION(BlueprintCallable)
    void SetBaseBuilding(const FTwinLinkNavSystemBuildingInfo& BaseBuilding) {
        this->BaseBuilding = BaseBuilding;
    }

    /*
     * @brief : レベルに登録されている建物リストを取得する
     */
    UFUNCTION(BlueprintCallable)
        TArray<FTwinLinkNavSystemBuildingInfo> GetBuildingInfo() const;

    UFUNCTION(BlueprintCallable)
        void Clear();
private:
    /*
     * @brief : PathFinderからパス検索準備完了時のコールバックとして登録する
     */
    void OnReadyPathFinding();

    /*
     * @brief : デバッグ描画実行
     */
    void DebugDraw();

    // -----------------------
    // Editor設定系
    // -----------------------
    // ランタイム時に利用するパラメータ
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        UTwinLinkNavSystemParam* RuntimeParam = nullptr;

    // ナビメッシュのバウンディングボリューム
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TObjectPtr<ANavMeshBoundsVolume> NavMeshBoundVolume;

    // 道路メッシュのコリジョンチャンネル
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TEnumAsByte<ECollisionChannel> DemCollisionChannel = ECollisionChannel::ECC_WorldStatic;

    // 道路メッシュのAabb
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        FBox DemCollisionAabb;

    // 開始地点の建物
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        FTwinLinkNavSystemBuildingInfo BaseBuilding;

private:
    // -----------------------
    // ランタイム系
    // -----------------------

    // 現在どのポイントを編集しているかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        NavSystemMode NowSelectedMode = NavSystemMode::Undefined;

    // パス描画のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        TArray<AUTwinLinkNavSystemPathDrawer*> PathDrawers;

    // パス探索のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        ATwinLinkNavSystemPathFinder* NowPathFinder = nullptr;

    // パス検索情報
    std::optional<FTwinLinkNavSystemFindPathInfo> PathFindInfo;

    // -----------------------
    // デバッグ系
    // -----------------------
    // デバッグ用) パス検索のデバッグ表示を行うかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        bool DebugCallPathFinding = false;
protected:
    virtual void BeginPlay() override;
};
