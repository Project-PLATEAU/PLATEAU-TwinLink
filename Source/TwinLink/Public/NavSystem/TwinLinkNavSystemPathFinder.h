// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkNavSystemBuildingInfo.h"
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

    // クリックしたときのイベント
    UDELEGATE(BlueprintAuthorityOnly)
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTwinLinkFacilityClicked, const FHitResult, HitResult, const FTwinLinkNavSystemBuildingInfo&, FacilityInfo);
public:
    DECLARE_MULTICAST_DELEGATE(OnReadyPathFindingDelegate);

    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathFinder();

    /*
     * @brief : パス検索を開始する
     */
    UFUNCTION(BlueprintCallable)
        virtual bool RequestStartPathFinding(FTwinLinkNavSystemFindPathInfo& Out);

    /*
     * @brief : 指定したタイプの位置を取得する. 存在しない場合はfalse
     */
    UFUNCTION(BlueprintCallable)
        bool TryGetPathLocation(TwinLinkNavSystemPathPointType Type, FVector& Out) const;

    /*
     * @brief : 指定したポイントの位置を取得
     */
    virtual std::optional<FVector> GetPathLocation(TwinLinkNavSystemPathPointType Type) const {
        return std::nullopt;
    }
    /*
     * @brief : 指定したポイントのマーカーのBoundingBoxを取得
     */
    virtual std::optional<FBox> GetPathLocationBox(TwinLinkNavSystemPathPointType Type) const {
        return std::nullopt;
    }

    /*
     * @brief : 指定したポイントの位置を設定
     */
    UFUNCTION(BlueprintCallable)
        virtual void SetPathLocation(TwinLinkNavSystemPathPointType Type, FVector Location) {}

    UFUNCTION(BlueprintCallable)
        virtual bool TryGetCameraLocationAndLookAt(const FVector& NowCameraLocation, FVector& OutLocation, FVector& OutLookAt) const;

    /*
     * @brief : 現在のパスからカメラの位置を変更する
     */
    UFUNCTION(BlueprintCallable)
        virtual void ChangeCameraLocation(float MoveSec) const;

    /*
     * @brief : パス検索の準備ができている(目的地が正しく選択されている)
     */
    virtual bool IsReadyPathFinding() const {
        return false;
    }

    /*
     * @brief : 状態の初期化
     */
    virtual void Clear() {}

    // パス検索が可能になったときに呼ばれる       
    OnReadyPathFindingDelegate OnReadyPathFinding;

    UPROPERTY(BlueprintAssignable)
        FOnTwinLinkFacilityClicked OnFacilityClicked;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // 
    void OnFacilityClick(const FHitResult& Info, const FTwinLinkNavSystemBuildingInfo& Facility);

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

    TwinLinkNavSystemPathPointTypeT GetNextCreatePointType() const {
        return NextCreatePointType;
    }

    void SetNowSelectedPointType(TwinLinkNavSystemPathPointTypeT Val) {
        NextCreatePointType = Val.GetEnumValue();
    }

    /*
     * @brief : パス検索の準備ができている(目的地が正しく選択されている)
     */
    virtual bool IsReadyPathFinding() const override;

    /*
     * @brief : 指定したポイントの位置を取得
     */
    virtual std::optional<FVector> GetPathLocation(TwinLinkNavSystemPathPointType Type) const override;

    /*
     * @brief : 指定したポイントの位置を設定
     */
    virtual void SetPathLocation(TwinLinkNavSystemPathPointType Type, FVector Location) override;

    /*
     * @brief : 状態の初期化
     */
    virtual void Clear() override;

    /*
     * @brief : 指定したポイントの位置のマーカー含めたスクリーン領域を取得
     */
    std::optional<FBox> GetPathLocationBox(TwinLinkNavSystemPathPointType Type) const override;

private:
    /*
     * @brief : 指定したポイントのアクターを生成して返す
     */
    ATwinLinkNavSystemPathLocator* GetOrSpawnActor(TwinLinkNavSystemPathPointType Type);

    // パス検索地点のBP
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TMap<TwinLinkNavSystemPathPointType, TSubclassOf<ATwinLinkNavSystemPathLocator>> PathLocatorBps;

    // 次生成予定の目的地タイプ
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        TwinLinkNavSystemPathPointType NextCreatePointType = TwinLinkNavSystemPathPointType::Start;

    // パス検索地点のアクター
    UPROPERTY(EditDefaultsOnly, Category = TwinLink_Path)
        TMap<TwinLinkNavSystemPathPointType, ATwinLinkNavSystemPathLocator*> PathLocatorActors;

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

    virtual void SetPathLocation(TwinLinkNavSystemPathPointType Type, FVector Location) override;

private:
    // 現在選択しているアクターのスクリーン位置オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        bool bDebugIsLocationMovable = false;
};
