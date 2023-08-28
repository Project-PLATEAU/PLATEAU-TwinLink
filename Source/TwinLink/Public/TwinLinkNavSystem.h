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
class APLATEAUInstancedCityModel;
class ANavMeshBoundsVolume;

/*
 * @brief : パス検索などランタイム中のナビメッシュ関係を管理するクラス.
 */
UCLASS()
class TWINLINK_API ATwinLinkNavSystem : public AActor{
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
     * @brief : 指定した2点間のパス検索を行う
     */
    TwinLinkNavSystemFindPathInfo RequestFindPath(const FVector& Start, const FVector& End) const;

    // 任意の２点間で
    void RequestFindPath();

    NavSystemPathPointTypeT GetNowSelectedPointType() const
    {
        return NowSelectedPointType;
    }

    void SetNowSelectedPointType(NavSystemPathPointTypeT Val)
    {
        NowSelectedPointType = Val.GetEnumValue();
    }

private:
    // Input設定
    void SetupInput();

    // Input Event(Action) イベントハンドラー関数
    void PressedAction();
    void ReleasedAction();

    // Input Event(Axis) イベントハンドラー関数
    void PressedAxis(const FInputActionValue& Value);

    // デバッグ描画実行
    void DebugDraw();

    // 道路メッシュのコリジョンチャンネル
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TEnumAsByte<ECollisionChannel> DemCollisionChannel = ECollisionChannel::ECC_WorldStatic;

    // 道路メッシュのAabb
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        FBox DemCollisionAabb;

    // 現在どのポイントを編集しているかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        NavSystemPathPointType NowSelectedPointType = NavSystemPathPointType::Start;

    // 現在どのポイントを編集しているかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        NavSystemMode NowSelectedMode = NavSystemMode::FindPathAnyPoint;

    // パス感覚の高さチェック
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        float FindPathHeightCheckInterval = 1000;

    // パス検索地点のBP
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
    TMap<NavSystemPathPointType, TSubclassOf<ATwinLinkNavSystemPathLocator>> PathLocatorBps;

    // パス描画のBP
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TSubclassOf<AUTwinLinkNavSystemPathDrawer> PathDrawerBp;

    // パス検索地点のアクター
    UPROPERTY(EditDefaultsOnly, Category = TwinLink_Path)
        TMap<NavSystemPathPointType, ATwinLinkNavSystemPathLocator*> PathLocatorActors;

    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
    ATwinLinkNavSystemPathLocator* NowSelectedPathLocatorActor;

    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        FVector2D NowSelectedPathLocatorActorScreenOffset = FVector2D::Zero();

    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        FVector NowSelectedPathLocatorActorLastValidLocation = FVector::Zero();

    // パス描画のアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        TArray<AUTwinLinkNavSystemPathDrawer*> PathDrawers;

    // ナビメッシュのバウンディングボリューム
    UPROPERTY(EditAnywhere, Category = Editor)
        TObjectPtr<ANavMeshBoundsVolume> NavMeshBoundVolume;

    // パス検索情報
    std::optional<TwinLinkNavSystemFindPathInfo> PathFindInfo;
    // -----------------------
    // デバッグ系
    // -----------------------
    // デバッグ用) パス検索のデバッグ表示を行うかどうか
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        bool DebugCallFindPath = false;

protected:
    virtual void BeginPlay() override;
};
