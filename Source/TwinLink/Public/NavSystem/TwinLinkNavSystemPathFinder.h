// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkNavSystemDef.h"
#include "TwinLinkNavSystemFindPathInfo.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemPathFinder.generated.h"

class ATwinLinkNavSystem;

UCLASS()
class TWINLINK_API ATwinLinkNavSystemPathFinder : public AActor {
    GENERATED_BODY()
public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathFinder();

    /*
     * @brief : パス検索を開始する
     */
    virtual bool RequestStartPathFinding(TwinLinkNavSystemFindPathInfo& Out);

    /*
     * @brief : 指定したポイントの位置を取得
     */
    virtual std::optional<FVector> GetPathLocation(NavSystemPathPointTypeT Type) const {
        return std::nullopt;
    }

    /*
     * @brief : パス検索の準備ができている(目的地が正しく選択されている)
     */
    virtual bool IsReadyPathFinding() const {
        return false;
    }
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
    TwinLinkNavSystemFindPathInfo RequestPathFinding(const FVector& Start, const FVector& End) const;
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
    virtual std::optional<FVector> GetPathLocation(NavSystemPathPointTypeT Type) const override;

private:
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
