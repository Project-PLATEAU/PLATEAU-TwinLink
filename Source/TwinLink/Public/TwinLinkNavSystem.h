// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationData.h"
#include "TwinLinkNavSystem.generated.h"
class APLATEAUInstancedCityModel;
class ANavMeshBoundsVolume;
/**
 * 
 */
UCLASS()
class TWINLINK_API ATwinLinkNavSystem : public AActor
{
	GENERATED_BODY()
public:
    ATwinLinkNavSystem();

    /*
     * @brief : パス検索
     */
    FPathFindingResult RequestFindPath(const FVector& Start, const FVector& End);

    /*
     * @brief : FindPathのリクエスト作成のラッパー
     */
    bool CreateFindPathRequest(const FVector& Start, const FVector& End, FPathFindingQuery& OutRequest);

    virtual void Tick(float DeltaSeconds) override;
private:
    // デバッグ用) パス検索のデバッグ表示を行うかどうか
    UPROPERTY(EditAnywhere, Category = Test)
    bool DebugCallFindPath = false;
    // デバッグ用) パス検索のデバッグ表示の際の描画オフセット
    UPROPERTY(EditAnywhere, Category = Test)
    float DebugFindPathUpOffset = 4000;

    FNavPathQueryDelegate DebugPathFindDelegate;
    // デバッグ用) スタート位置
    FWeakObjectPtr DebugFindPathStart;
    // デバッグ用) 目的地
    FWeakObjectPtr DebugFindPathEnd;
    // デバッグ描画実行
    void DebugDraw();
};
