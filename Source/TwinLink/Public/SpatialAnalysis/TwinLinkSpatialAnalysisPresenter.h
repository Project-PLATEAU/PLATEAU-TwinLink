// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkSpatialID.h"
#include "GameFramework/Actor.h"
#include "TwinLinkSpatialAnalysisPresenter.generated.h"

class ATwinLinkCityObjectTree;
class ATwinLinkNavSystemPathLocator;
class ATwinLinkNavSystem;



UCLASS()
class TWINLINK_API ATwinLinkSpatialAnalysisPresenter : public AActor {
    GENERATED_BODY()
public:

    UDELEGATE(BlueprintAuthorityOnly)
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpatialIdChangedDelegate, const FTwinLinkSpatialID&, Before, const FTwinLinkSpatialID&, After);
public:
    // Sets default values for this actor's properties
    ATwinLinkSpatialAnalysisPresenter();

public:
    UPROPERTY(BlueprintAssignable)
        FOnSpatialIdChangedDelegate OnSpatialIdChanged;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
        bool IsValidSpatialId() const;

    UFUNCTION(BlueprintCallable)
        void SetZoom(int Value);

    const ATwinLinkCityObjectTree* GetTree() const;

    // WorldViewerのイベントコールバック
    void OnClicked(const FHitResult& Info);

    //　描画処理
    void DrawUpdate(float DeltaTime);

    // 現在選択中の空間ID
    UFUNCTION(BlueprintCallable)
        bool TryGetNowSpatialId(FTwinLinkSpatialID& Out) const;

    // 現在選択中の空間ID
    std::optional<FTwinLinkSpatialID> GetNowSpatialId() const;
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
};
