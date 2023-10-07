// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkNavSystemBuildingInfo.h"
#include "TwinLinkNavSystemDef.h"
#include "TwinLinkNavSystemFindPathInfo.h"
#include "TwinLinkNavSystemPathDrawer.h"
#include "TwinLinkNavSystemParam.h"
#include "TwinLinkNavSystemFindPathUiInfo.h"
#include "TwinLinkNavSystem.generated.h"
class UPLATEAUCityObjectGroup;
class UNavigationSystemV1;
class UTwinLinkFacilityInfo;
class UCameraComponent;
class ATwinLinkNavSystemPathFinder;
class APLATEAUInstancedCityModel;
class ANavMeshBoundsVolume;
class ATwinLinkWorldViewer;
class APlayerCameraManager;

DECLARE_EVENT_TwoParams(ATwinLinkNavSystem, OnFacilityClickedDelegate, const FHitResult&, const FTwinLinkNavSystemBuildingInfo&);
/*
 * @brief : パス検索などランタイム中のナビメッシュ関係を管理するクラス.
 */
UCLASS()
class TWINLINK_API ATwinLinkNavSystem : public AActor {
    GENERATED_BODY()
public:
    ATwinLinkNavSystem();

    /*
     * @brief : Worldに配置されているATwinLinkNavSystemをとってくる. 毎回同じこと書くの面倒なので
     */
    static ATwinLinkNavSystem* GetInstance(const UWorld* World);

    /*
     * @brief : Worldに配置されているATwinLinkWorldViewerをとってくる. 毎回同じこと書くの面倒なので
     */
    static ATwinLinkWorldViewer* GetWorldViewer(const UWorld* World);

    /*
     * @brief : Worldに配置されているAPlayerCameraManagerをとってくる. 毎回同じこと書くの面倒なので
     */
    static APlayerCameraManager* GetPlayerCameraManager(const UWorld* World);

    /*
     * @brief : Worldに配置されているUTwinLinkFacilityInfoSystemをとってくる. 毎回同じこと書くの面倒なので
     */
    static UTwinLinkFacilityInfoSystem* GetFacilityInfoSystem(const UWorld* World);

    static bool FindNavMeshPoint(const UNavigationSystemV1* NavSys, const UStaticMeshComponent* StaticMeshComp, FVector& OutPos);
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

    // 建物クリックしたときのコールバック
    OnFacilityClickedDelegate OnFacilityClicked;

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
        FTwinLinkNavSystemFindPathUiInfo GetDrawMoveTimeUiInfo(TwinLinkNavSystemMoveType MoveType, const FBox2D& ScreenRange) const;

    /*
     * @brief : 現在のパス検索モードアクタ
     */
    UFUNCTION(BlueprintCallable)
        ATwinLinkNavSystemPathFinder* GetNowPathFinder();

    UFUNCTION(BlueprintCallable)
        FTwinLinkNavSystemBuildingInfo GetBaseBuilding() const;

    /*
     * @brief : レベルに登録されている建物リストを取得する
     */
    UFUNCTION(BlueprintCallable)
        TArray<FTwinLinkNavSystemBuildingInfo> GetBuildingInfos() const;

    UFUNCTION(BlueprintCallable)
        void Clear();

    UFUNCTION(BlueprintCallable)
        FVector2D GetDebugHudPosition() const {
        return DebugHudPosition;
    }

    /*
     * @brief : 有効かどうか
     * @param : includePathFinder : NowPathFinder含めて有効かどうか
     */
    UFUNCTION(BlueprintCallable)
        bool IsValid() const;
private:
    /*
     * @brief : PathFinderからパス検索準備完了時のコールバックとして登録する
     */
    void OnReadyPathFinding();

    /*
     * @brief : WorldViewerで建物クリックしたときに呼ばれる
     */
    void OnFacilityClick(FHitResult Info) const;

    /*
     * @brief : デバッグ描画実行
     */
    void DebugDraw();

    /*
     * @brief : デバッグ用のBeginPlay
     */
    void DebugBeginPlay();

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

    // 建物情報のマップキャッシュ
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TMap<FString, TWeakObjectPtr<UPLATEAUCityObjectGroup>> BuildingMap;
private:
    // -----------------------
    // ランタイム系
    // -----------------------

    // 現在どのポイントを編集しているかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Runtime)
        NavSystemMode NowSelectedMode = NavSystemMode::Undefined;

    // パス描画のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Runtime)
        TArray<AUTwinLinkNavSystemPathDrawer*> PathDrawers;

    // パス探索のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Runtime)
        ATwinLinkNavSystemPathFinder* NowPathFinder = nullptr;

    UPROPERTY(EditAnywhere, Category = TwinLink_Runtime)
        UTwinLinkFacilityInfo* BaseBuildingInfo = nullptr;
    // パス検索情報
    std::optional<FTwinLinkNavSystemFindPathInfo> PathFindInfo;

    // -----------------------
    // デバッグ系
    // -----------------------
    // デバッグ用) パス検索のデバッグ表示を行うかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        bool DebugCallPathFinding = false;

    // デバッグ用) 仮の建物リストを使うかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        bool DebugUseDebugBuildings = false;

    // パス探索のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        FVector2D DebugHudPosition = FVector2D::Zero();

    //    UTwinLinkFacilityInfo DebugStartBuilding;
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        TArray<UTwinLinkFacilityInfo*> DebugFacilityInfos;
protected:
    virtual void BeginPlay() override;
};
