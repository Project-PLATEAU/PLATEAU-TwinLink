// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkNavSystemDef.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemPathLocator.generated.h"

class UTwinLinkFacilityInfo;
class UNavigationSystemV1;

UCLASS()
class TWINLINK_API ATwinLinkNavSystemPathLocator : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathLocator();
protected:
    virtual void BeginPlay() override;
public:
    virtual void Tick(float DeltaSeconds) override;

    // 現在の状態
    UFUNCTION(BlueprintCallable)
        TwinLinkNavSystemPathLocatorState GetNowState() const;

    // 位置更新する
    bool UpdateLocation(const UNavigationSystemV1* NavSys, const FHitResult& HitResult);

    // 位置更新する
    virtual bool UpdateLocation(const UNavigationSystemV1* NavSys, const FVector& Location);

    // 選択状態にする
    void Select();

    // 非選択状態にする
    void UnSelect();

    // 最後の有効な位置を取得
    std::optional<FVector> GetLastValidLocation() const;

    virtual ECollisionChannel GetCollisionChannel() const;

protected:
    // 現在の状態
    UPROPERTY(EditAnywhere, Category = TwinLink_Base)
        TwinLinkNavSystemPathLocatorState State = TwinLinkNavSystemPathLocatorState::Undefined;


    // 現在の状態
    UPROPERTY(EditAnywhere, Category = TwinLink_Base)
        bool IsSelected = false;

    // 最後にValid状態になった位置
    std::optional<FVector> LastValidLocation = std::nullopt;
};
