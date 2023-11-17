// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkPeopleFlowSystem.h"
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

    UDELEGATE(BlueprintAuthorityOnly)
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangePeopleFlowDelegate, const FTwinLinkPopulationData&, Data);

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
    // 空間ID変更チェック
    void CheckSpatialIdChanged(const std::optional<FTwinLinkSpatialID>& Before) const;

    // 4分木取得
    const ATwinLinkCityObjectTree* GetTree() const;

    // WorldViewerのイベントコールバック
    void OnClicked(const FHitResult& Info);

    //　描画処理
    void DrawUpdate(float DeltaTime) const;

    // デバッグ描画
    void DebugDrawUpdate(float DeltaTime) const;

    // 人流データレスポンス受信
    UFUNCTION()
        void OnReceivedPeopleFlowResponse(const FTWinLinkPeopleFlowApiResult& Result);
public:
    UPROPERTY(BlueprintAssignable)
        FOnSpatialIdChangedDelegate OnSpatialIdChanged;

    UPROPERTY(BlueprintAssignable)
        FOnChangePeopleFlowDelegate OnChangePeopleFlow;

    UFUNCTION(BlueprintPure)
        bool IsValidSpatialId() const;

    UFUNCTION(BlueprintCallable)
        void SetZoom(int Value);

    // 現在選択中の空間ID取得
    UFUNCTION(BlueprintPure)
        bool TryGetNowSpatialId(FTwinLinkSpatialID& Out) const;

    UFUNCTION(BlueprintCallable)
        bool RequestPeopleFlow(FDateTime DateTime);

private:
    // マウスヒット位置
    UPROPERTY(VisibleAnywhere)
        FHitResult NowHitResult;

    // ズームレベル
    UPROPERTY(EditAnywhere)
        int Zoom = 20;

    // 最後に人流データをリクエストした時間
    UPROPERTY(EditAnywhere)
        FDateTime LastPeopleFlowRequestedTime;

    // 人流データ取得API
    UPROPERTY(VisibleAnywhere)
        UTwinLinkPeopleFlowApi* PeopleFlowApi = nullptr;

    // 毎回検索するの重そうなのでキャッシュ
    TWeakObjectPtr<ATwinLinkCityObjectTree> CacheTree;

    // デバッグ表示-------------------
    // 現在選択中の空間ID
    UPROPERTY(VisibleAnywhere)
        FTwinLinkSpatialID DebugNowSelectedId;

    // デバッグ表示
    UPROPERTY(EditAnywhere)
        bool DebugShowSpace;
};
