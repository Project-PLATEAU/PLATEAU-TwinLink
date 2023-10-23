// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkSpatialID.h"
#include "GameFramework/Actor.h"
#include "TwinLinkSpatialAnalysisUiInfo.h"
#include "TwinLinkSpatialAnalysisPresenter.generated.h"

class ATwinLinkCityObjectTree;
class ATwinLinkNavSystemPathLocator;
class ATwinLinkNavSystem;


UCLASS()
class TWINLINK_API ATwinLinkSpatialAnalysisPresenter : public AActor {
    GENERATED_BODY()
public:

    UDELEGATE(BlueprintAuthorityOnly)
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpatialIdChangedDelegate, const FTwinLinkSpatialID&, Before, const FTwinLinkSpatialAnalysisUiInfo&, After);
public:
    // Sets default values for this actor's properties
    ATwinLinkSpatialAnalysisPresenter();

    // 現在選択中の空間ID取得
    std::optional<FTwinLinkSpatialID> GetNowSpatialId() const;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;
private:
    void CheckSpatialIdChanged(const std::optional<FTwinLinkSpatialID>& Before) const;

    const ATwinLinkCityObjectTree* GetTree() const;

    // WorldViewerのイベントコールバック
    void OnClicked(const FHitResult& Info);

    //　描画処理
    void DrawUpdate(float DeltaTime) const;

    // デバッグ描画
    void DebugDrawUpdate(float DeltaTime) const;
public:
    UPROPERTY(BlueprintAssignable)
        FOnSpatialIdChangedDelegate OnSpatialIdChanged;

    UFUNCTION(BlueprintPure)
        bool IsValidSpatialId() const;

    UFUNCTION(BlueprintCallable)
        void SetZoom(int Value);

    // 現在選択中の空間ID取得
    UFUNCTION(BlueprintPure)
        bool TryGetNowSpatialId(FTwinLinkSpatialID& Out) const;

private:
    // マウスヒット位置
    UPROPERTY(VisibleAnywhere)
        FHitResult NowHitResult;

    // ズームレベル
    UPROPERTY(EditAnywhere)
        int Zoom = 20;

    TWeakObjectPtr<ATwinLinkCityObjectTree> CacheTree;

    // 現在選択中の空間ID
    UPROPERTY(VisibleAnywhere)
        FTwinLinkSpatialID DebugNowSelectedId;

    // デバッグ表示
    UPROPERTY(EditAnywhere)
        bool DebugShowSpace;
};
