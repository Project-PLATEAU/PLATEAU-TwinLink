// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkNavSystemDef.h"
#include "TwinLinkNavSystemFindPathInfo.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemPathFinder.generated.h"

class ATwinLinkNavSystemPathLocator;
class ATwinLinkNavSystem;

UCLASS()
class TWINLINK_API ATwinLinkNavSystemPathFinder : public AActor {
    GENERATED_BODY()
public:
    DECLARE_MULTICAST_DELEGATE(OnReadyPathFindingDelegate);

    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathFinder();

    /*
     * @brief : パス検索を開始する
     */
    UFUNCTION(BlueprintCallable)
    virtual bool RequestStartPathFinding(FTwinLinkNavSystemFindPathInfo& Out);

    UFUNCTION(BlueprintCallable)
    bool TryGetPathLocation(NavSystemPathPointType Type, FVector& Out);

    /*
     * @brief : 指定したポイントの位置を取得
     */
    virtual std::optional<FVector> GetPathLocation(NavSystemPathPointType Type) const {
        return std::nullopt;
    }

    /*
     * @brief : 指定したポイントの位置を設定
     */
    UFUNCTION(BlueprintCallable)
    virtual void SetPathLocation(NavSystemPathPointType Type, FVector Location) {}

    /*
     * @brief : パス検索の準備ができている(目的地が正しく選択されている)
     */
    virtual bool IsReadyPathFinding() const {
        return false;
    }

    /*
     * @brief : 状態の初期化
     */
    virtual void Clear(){}

    // パス検索が可能になったときに呼ばれる       
    OnReadyPathFindingDelegate OnReadyPathFinding;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // TwinLinkNavSystem
    const ATwinLinkNavSystem* GetTwinLinkNavSystem() const;
private:
    /*
     * @brief : 指定した2点間のパス検索を行う
     */
    FTwinLinkNavSystemFindPathInfo RequestPathFinding(const FVector& Start, const FVector& End) const;

};

UCLASS()
class TWINLINK_API ATwinLinkNavSystemPathFinderAnyLocation : public ATwinLinkNavSystemPathFinder {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathFinderAnyLocation();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    NavSystemPathPointTypeT GetNextCreatePointType() const {
        return NextCreatePointType;
    }

    void SetNowSelectedPointType(NavSystemPathPointTypeT Val) {
        NextCreatePointType = Val.GetEnumValue();
    }

    /*
     * @brief : パス検索の準備ができている(目的地が正しく選択されている)
     */
    virtual bool IsReadyPathFinding() const override;

    /*
     * @brief : 指定したポイントの位置を取得
     */
    virtual std::optional<FVector> GetPathLocation(NavSystemPathPointType Type) const override;

    /*
     * @brief : 指定したポイントの位置を設定
     */
    virtual void SetPathLocation(NavSystemPathPointType Type, FVector Location) override;

    /*
     * @brief : 状態の初期化
     */
    virtual void Clear() override;

private:
    /*
     * @brief : 指定したポイントのアクターを生成して返す
     */
    ATwinLinkNavSystemPathLocator* GetOrSpawnActor(NavSystemPathPointType Type);

    // パス検索地点のBP
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TMap<NavSystemPathPointType, TSubclassOf<ATwinLinkNavSystemPathLocator>> PathLocatorBps;

    // 次生成予定の目的地タイプ
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        NavSystemPathPointType NextCreatePointType = NavSystemPathPointType::Start;

    // パス検索地点のアクター
    UPROPERTY(EditDefaultsOnly, Category = TwinLink_Path)
        TMap<NavSystemPathPointType, ATwinLinkNavSystemPathLocator*> PathLocatorActors;

    // 現在選択しているアクター
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        ATwinLinkNavSystemPathLocator* NowSelectedPathLocatorActor;
    // 現在選択しているアクターのスクリーン位置オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        FVector2D NowSelectedPathLocatorActorScreenOffset = FVector2D::Zero();
};

UCLASS()
class TWINLINK_API ATwinLinkNavSystemPathFinderListSelect : public ATwinLinkNavSystemPathFinderAnyLocation {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathFinderListSelect();

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

private:
    // 現在選択しているアクターのスクリーン位置オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        bool bDebugIsLocationMovable = false;
};
