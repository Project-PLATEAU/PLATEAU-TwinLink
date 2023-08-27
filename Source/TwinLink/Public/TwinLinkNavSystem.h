// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"
#include "InputMappingContext.h" 
#include "InputAction.h"
#include "InputActionValue.h"
#include "TwinLinkNavSystemDef.h"
#include "TwinLinkNavSystemFindPathInfo.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "TwinLinkNavSystem.generated.h"
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
     * @brief : 指定した2点間のパス検索を行う
     */
    TwinLinkNavSystemFindPathInfo RequestFindPath(const FVector& Start, const FVector& End) const;

    // 任意の２点間で
    void RequestFindPath();

    // 開始地点取得
    FVector GetStartPoint() const;

    // 開始地点設定
    void SetStartPoint(const FVector& V);

    // 目的地取得
    FVector GetDestPoint() const;

    // 目的地設定
    void SetDestPoint(const FVector& V);

    // 開始地点のアクター
    AActor* GetStartActor();

    // 開始地点のアクター
    const AActor* GetStartActor() const;

    // 目的地点のアクター
    AActor* GetDestActor();

    // 目的地点のアクター
    const AActor* GetDestActor() const;

    AActor* GetNowSelectedPointActor();
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
private:
    // 道路メッシュのコリジョンチャンネル
    UPROPERTY(EditAnywhere, Category = Editor)
        TEnumAsByte<ECollisionChannel> DemCollisionChannel = ECollisionChannel::ECC_GameTraceChannel1;

    // 道路メッシュのAabb
    UPROPERTY(EditAnywhere, Category = Editor)
        FBox DemCollisionAabb;

    // 現在どのポイントを編集しているかどうか
    UPROPERTY(EditAnywhere, Category = Path)
        NavSystemPathPointType NowSelectedPointType = NavSystemPathPointType::Start;

    // 現在どのポイントを編集しているかどうか
    UPROPERTY(EditAnywhere, Category = Path)
        NavSystemMode NowSelectedMode = NavSystemMode::FindPathAnyPoint;

    // パス感覚の高さチェック
    UPROPERTY(EditAnywhere, Category = Path)
        float FindPathHeightCheckInterval = 1000;

    /** Called for Action input */
    void EventAction(const FInputActionValue& Value);

    /** Called for Axis input */
    void EventAxis(const FInputActionValue& Value);

    /** MappingContext */
    UPROPERTY(EditAnywhere, Category = Input)
        class UInputMappingContext* DefaultMappingContext;

    /** Action Input */
    UPROPERTY(EditAnywhere, Category = Input)
        class UInputAction* ActionInput;

    /** Axis Input */
    UPROPERTY(EditAnywhere, Category = Input)
        class UInputAction* AxisInput;

    UPROPERTY(EditAnywhere, Category = Editor)
        TSubclassOf<class AActor> PathLocatorStartBp;

    UPROPERTY(EditAnywhere, Category = Editor)
        TSubclassOf<class AActor> PathLocatorDestBp;


    UPROPERTY(EditAnywhere, Category = Editor)
        TSubclassOf<class AActor> PathLineBp;


    // デバッグ用) パス検索のデバッグ表示を行うかどうか
    UPROPERTY(EditAnywhere, Category = Test)
        bool DebugCallFindPath = false;
    // デバッグ用) パス検索のデバッグ表示の際の描画オフセット
    UPROPERTY(EditAnywhere, Category = Test)
        float DebugFindPathUpOffset = 10;

    // パス検索情報
    std::optional<TwinLinkNavSystemFindPathInfo> PathFindInfo;

    FNavPathQueryDelegate DebugPathFindDelegate;
    // スタート位置
    FWeakObjectPtr FindPathStartActor;
    // 目的地
    FWeakObjectPtr FindPathDestActor;

    /*
    // パス描画用のLine
    UPROPERTY(EditAnywhere, Category = Editor)
    TArray<USplineMeshComponent*> FindPathLineActorArray;

    UPROPERTY(EditAnywhere, Category = Editor)
        USplineComponent* PathLineComponent;
        */
    // ナビメッシュのバウンディングボリューム
    UPROPERTY(EditAnywhere, Category = Editor)
        TObjectPtr<ANavMeshBoundsVolume> NavMeshBoundVolume;
protected:
    virtual void BeginPlay() override;
};
