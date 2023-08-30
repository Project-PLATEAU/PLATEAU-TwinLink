// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkNavSystemDef.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemPathLocator.generated.h"

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
    NavSystemPathLocatorState GetNowState() const {
        return State;
    }

    // 位置更新する
    void UpdateLocation(const UNavigationSystemV1* NavSys, const FHitResult& HitResult);

    // 選択状態にする
    void Select();

    // 非選択状態にする
    void UnSelect();

protected:
    // 現在の状態
    UPROPERTY(EditAnywhere, Category = TwinLink_Base)
        NavSystemPathLocatorState State = NavSystemPathLocatorState::Undefined;


    // 現在の状態
    UPROPERTY(EditAnywhere, Category = TwinLink_Base)
        bool IsSelected = false;

    // 最後にValid状態になった位置
    std::optional<FVector> LastValidLocation = FVector::Zero();
};
