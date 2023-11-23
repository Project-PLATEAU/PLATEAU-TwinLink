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
        virtual void DrawPath(const TArray<FVector>& PathPoints, float DeltaSeconds) {};
};

UCLASS(Blueprintable)
class TWINLINK_API AUTwinLinkNavSystemPathDrawerNiagara : public AUTwinLinkNavSystemPathDrawer {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    AUTwinLinkNavSystemPathDrawerNiagara();

    virtual void DrawPath(const TArray<FVector>& PathPoints, float DeltaSeconds) override;;

private:

    // 描画ポイント
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        float DrawPointInterval = 10;

    // 高さ方向の描画オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        float DrawPointHeightOffset = 10;
};

/*
 * 矢印表示
 */
UCLASS(Blueprintable)
class TWINLINK_API AUTwinLinkNavSystemPathDrawerArrow : public AUTwinLinkNavSystemPathDrawer {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    AUTwinLinkNavSystemPathDrawerArrow();

    virtual void BeginPlay() override;

    virtual void DrawPath(const TArray<FVector>& PathPoints, float DeltaSeconds) override;

    virtual void Tick(float DeltaSeconds) override;
private:
    UActorComponent* CreateChildArrow();
    /*
     * @brief : 矢印の位置更新する
     * bInterpolate : 回転時に補間する
     */
    bool UpdateMatrix(float DeltaSeconds, bool bInterpolate);;
private:

    // 矢印は最低これだけは離して描画するように、表示数を制限する
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        float DrawPointInterval = 10000;

    // 表示する最大の矢印の数
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        int MaxArrowNum = 10;

    // 高さ方向の描画オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        float DrawPointHeightOffset = 10;

    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TSubclassOf<UStaticMeshComponent> ArrowTemplate;

    // 生成された画像イメージ
    UPROPERTY(VisibleAnywhere, Category = TwinLink_Runtime)
        TArray<UStaticMeshComponent*> ArrowComps;

    // パスの描画ポイント
    UPROPERTY(VisibleAnywhere, Category = TwinLink_Runtime)
        TArray<FVector> DrawPathPoints;

    // 移動スピード
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        float MoveSpeed = 10.f;

    // パスの移動は最低この時間はかかるようにする[s]
    // パスが短い時は遅くさせたいので
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        float MinMoveSec = 3.f;

    // 回転スピード
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        float RotSpeedCoef = 10.f;

    // 現在の移動量
    UPROPERTY(VisibleAnywhere)
        float NowPosition = 0.f;

    // ルートの長さ
    UPROPERTY(VisibleAnywhere, Category = TwinLink_Runtime)
        float TotalPathDistance;

};

UCLASS(Blueprintable)
class TWINLINK_API AUTwinLinkNavSystemPathDrawerDebug : public AUTwinLinkNavSystemPathDrawer {
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    AUTwinLinkNavSystemPathDrawerDebug();

    virtual void DrawPath(const TArray<FVector>& PathPoints, float DeltaSec) override;;

    virtual void Tick(float DeltaSeconds) override;
private:
    // 描画ポイント
    UPROPERTY(EditDefaultsOnly, Category = TwinLink_Test)
        TArray<FVector> DebugPathPoints;

    // パス検索のデバッグ表示の際の描画オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Test)
        float DebugFindPathUpOffset = 10;
};