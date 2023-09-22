// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "TwinLinkNavSystemPathDrawer.generated.h"


UCLASS(Blueprintable)
class TWINLINK_API AUTwinLinkNavSystemPathDrawer : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    AUTwinLinkNavSystemPathDrawer();
    UFUNCTION(BlueprintCallable)
        virtual void DrawPath(const TArray<FVector>& PathPoints) {};
};

UCLASS(Blueprintable)
class TWINLINK_API AUTwinLinkNavSystemPathDrawerNiagara : public AUTwinLinkNavSystemPathDrawer {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    AUTwinLinkNavSystemPathDrawerNiagara();

    virtual void DrawPath(const TArray<FVector>& PathPoints) override;;

private:

    // 描画ポイント
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        float DrawPointInterval = 10;

    // 高さ方向の描画オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        float DrawPointHeightOffset = 10;
};

UCLASS(Blueprintable)
class TWINLINK_API AUTwinLinkNavSystemPathDrawerDebug : public AUTwinLinkNavSystemPathDrawer {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    AUTwinLinkNavSystemPathDrawerDebug();

    virtual void DrawPath(const TArray<FVector>& PathPoints) override;;

    virtual void Tick(float DeltaSeconds) override;
private:
    // 描画ポイント
    UPROPERTY(EditDefaultsOnly, Category = TwinLink_Test)
        TArray<FVector> DebugPathPoints;

    // パス検索のデバッグ表示の際の描画オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        float DebugFindPathUpOffset = 10;
};