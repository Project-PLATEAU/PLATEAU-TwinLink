// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkNavSystemPathLocator.h"
#include "Components/Widget.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemEntranceLocator.generated.h"

class ATwinLinkNavSystemPathLocator;
class ATwinLinkNavSystem;

/*
 * EntranceLocatorの参照ノード. EntranceLocatorはSingleton前提
 */
class TWINLINK_API FTwinLinkEntranceLocatorNode {
    static TArray<FTwinLinkEntranceLocatorNode*>& AllNodes();
public:
    static bool IsAnyNodeVisible();
    FTwinLinkEntranceLocatorNode();

    virtual ~FTwinLinkEntranceLocatorNode();
    virtual bool IsVisibleEntranceLocator() const = 0;

    virtual const UWorld* GetWorld() const = 0;

    std::optional<FVector> GetEntranceLocation() const;
    bool SetDefaultEntranceLocation(const FString& FeatureId, bool bForce = false) const;
    bool SetEntranceLocation(const UTwinLinkFacilityInfo* Info, bool bForce = false) const;
};

class TWINLINK_API FTwinLinkEntranceLocatorWidgetNode : public FTwinLinkEntranceLocatorNode {
public:
    FTwinLinkEntranceLocatorWidgetNode(UWidget* W);
    virtual bool IsVisibleEntranceLocator() const override;
    virtual const UWorld* GetWorld() const override;

private:
    TWeakObjectPtr<UWidget> Widget = nullptr;
};

/*
 * EntranceLocatorの実態はTwinLinkNavSystemが持っている
 */
UCLASS()
class TWINLINK_API ATwinLinkNavSystemEntranceLocator : public ATwinLinkNavSystemPathLocator {
    GENERATED_BODY()
public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemEntranceLocator();

    virtual ~ATwinLinkNavSystemEntranceLocator() override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    /*
     * @brief : パス検索の準備ができている(目的地が正しく選択されている)
     */
    virtual bool IsValidLocation() const;

    virtual ECollisionChannel GetCollisionChannel() const override;

    using ATwinLinkNavSystemPathLocator::UpdateLocation;
    // 位置更新する
    virtual bool UpdateLocation(const UNavigationSystemV1* NavSys, const FVector& Location) override;
    /*
     * @brief : 指定したポイントの位置を取得
     */
    UFUNCTION(BlueprintCallable)
        bool TryGetLocation(FVector& Out) const;

    std::optional<FVector> GetPathLocation() const;

    /*
     * @brief : 指定したポイントの位置を設定.
     */
    UFUNCTION(BlueprintCallable)
        void SetEntranceLocation(const UTwinLinkFacilityInfo* Info);

    /*
     * @brief : 指定したポイントの位置を設定.
     */
    UFUNCTION(BlueprintCallable)
        void SetDefaultEntranceLocation(const FString& FeatureId);

private:
    // 現在選択しているアクターのスクリーン位置オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        FVector2D NowSelectedPathLocatorActorScreenOffset = FVector2D::Zero();
};