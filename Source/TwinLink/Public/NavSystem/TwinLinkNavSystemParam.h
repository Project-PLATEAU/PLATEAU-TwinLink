// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkNavSystemDef.h"
#include "Engine/DataAsset.h"
#include "TwinLinkNavSystemParam.generated.h"

class AUTwinLinkNavSystemPathDrawer;
class ATwinLinkNavSystemPathFinder;
/**
 *
 */
UCLASS()
class TWINLINK_API UTwinLinkNavSystemParam : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
        float GetMoveSpeedKmPerH(TwinLinkNavSystemMoveType MoveType) const;
public:
    UPROPERTY(EditAnywhere, meta = (Comment = "ワールド座標における1[m]のサイズ"), Category = "Path")
        float WorldUnitMeter = 100.f;

    UPROPERTY(EditAnywhere, meta = (Comment = "利用者の歩く速度[km/h]"), Category = "Path")
        float WalkSpeedKmPerH = 10.f;

    UPROPERTY(EditAnywhere, meta = (Comment = "利用者の車の速度[km/h]"), Category = "Path")
        float CarSpeedKmPerH = 10.f;

    UPROPERTY(EditAnywhere, meta = (Comment = "経路探索時開始時の開始地点からのオフセット(X:Θ, Y:φ, Z:距離)"), Category = "Path")
        FVector RouteGuideStartCameraRotDist = FVector(30, 85, 10000);

    UPROPERTY(EditAnywhere, meta = (Comment = "パスの点線描画の間隔"), Category = "Path")
        float PathPointInterval = 1000;

    UPROPERTY(EditAnywhere, meta = (Comment = "パス描画アクターのBP"), Category = "Path")
        TSubclassOf<AUTwinLinkNavSystemPathDrawer> PathDrawerBp;

    UPROPERTY(EditAnywhere, meta = (Comment = "パス探索のBPアクターのBP"), Category = "Path")
        TMap<NavSystemMode, TSubclassOf<ATwinLinkNavSystemPathFinder>> PathFinderBp;

    UPROPERTY(EditAnywhere, meta = (Comment = "カメラ補完時の係数"), Category = "Camera")
        float CameraLerpCoef = 1.f;
};
