// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"
#include "InputActionValue.h"
#include "TwinLinkNavSystemDef.h"
#include "TwinLinkNavSystemFindPathInfo.h"
#include "TwinLinkNavSystemPathDrawer.h"
#include "TwinLinkNavSystemPathLocator.h"
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

    /*
     * @brief : FindPathのリクエスト作成のラッパー
     */
    bool CreateFindPathRequest(const FVector& Start, const FVector& End, FPathFindingQuery& OutRequest) const;

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
        return PathFindingHeightCheckInterval;
    }

    /*
     * @brief : モード切替
     */
    UFUNCTION(BlueprintCallable)
        void ChangeMode(NavSystemMode Mode, bool bForce = false);
private:
    // Input設定
    void SetupInput();

    // デバッグ描画実行
    void DebugDraw();
    /*
     * Editor系
     */
     // ナビメッシュのバウンディングボリューム
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TObjectPtr<ANavMeshBoundsVolume> NavMeshBoundVolume;

    // 道路メッシュのコリジョンチャンネル
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TEnumAsByte<ECollisionChannel> DemCollisionChannel = ECollisionChannel::ECC_WorldStatic;

    // 道路メッシュのAabb
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        FBox DemCollisionAabb;

    // パス描画のBP
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TSubclassOf<AUTwinLinkNavSystemPathDrawer> PathDrawerBp;

    // パス探索のBP
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TMap<NavSystemMode, TSubclassOf<ATwinLinkNavSystemPathFinder>> PathFinderBp;

    /*
     * ランタイム系
     */
     // 現在どのポイントを編集しているかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        NavSystemMode NowSelectedMode = NavSystemMode::FindPathAnyPoint;

    // パスの高さチェックを行う間隔
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        float PathFindingHeightCheckInterval = 1000;

    // パス描画のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        TArray<AUTwinLinkNavSystemPathDrawer*> PathDrawers;

    // パス探索のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        ATwinLinkNavSystemPathFinder* NowPathFinder = nullptr;

    // パス検索情報
    std::optional<TwinLinkNavSystemFindPathInfo> PathFindInfo;

    // -----------------------
    // デバッグ系
    // -----------------------
    // デバッグ用) パス検索のデバッグ表示を行うかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        bool DebugCallPathFinding = false;
protected:
    virtual void BeginPlay() override;
};
