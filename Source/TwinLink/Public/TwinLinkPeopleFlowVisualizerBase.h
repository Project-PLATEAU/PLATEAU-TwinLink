// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TwinLinkPeopleFlowVisualizerBase.generated.h"

class FPLATEAUGeoReference;
class APLATEAUInstancedCityModel;

struct TestSpatialData {
    int X;
    int Y;
    int Z;
    int F;
    int TimeStamp;
    int Unit;
    int PeopleFlow;
};

/**
 * @brief 人流データの可視化機能を提供するコンポーネント
 * 仮でテスト処理も実装中
*/
UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINLINK_API UTwinLinkPeopleFlowVisualizerBase : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTwinLinkPeopleFlowVisualizerBase();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void TestFunc();

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FVector TestFuncProjectSpatialID(APLATEAUInstancedCityModel* CityModel);

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void TestFuncSpatialID(APLATEAUInstancedCityModel* CityModel);

    /**
     * @brief 空間IDを元に情報を要求する
     * 仮実装
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestInfoFromSpatialID();


    /**
     * @brief サンプル用のヒートマップ用のテクスチャを作成
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category="TwinLink")
    UTexture* CreateSampleHeatmapTexture();

    /**
     * @brief ヒートマップ用のテクスチャを作成
     * @param Width 幅
     * @param Height 高さ
     * @param HeatLevels 0-1のヒートマップの強度が格納された配列
     *  assert(HeatLevels.Num() == Width*Height)
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    UTexture* CreateHeatmapTexture(int Width, int Height, const TArray<float>& HeatLevels);

    /**
     *  未テスト ←動作を確認したらこの行を消す
     * 
     * @brief ヒートマップ用のテクスチャを作成
     * @param Width 幅
     * @param Height 高さ
     * @param MinAmount 量の最小値
     * @param MaxAmount 量の最大値
     * @param Amounts 量が格納された配列
     *  assert(HeatLevels.Num() == Width*Height)
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    UTexture* CreateHeatmapTextureByAmount(int Width, int Height, 
        int MinAmount, int MaxAmount, const TArray<int>& Amounts);

private:
    /**
     * @brief 人流データの更新を行う
     * @param DataArray 
    */
    void UpdatePeopleFlow(const TArray<TestSpatialData>& DataArray);

private:
    /** 人流データの最小値 **/
    int PeopleFlowMinAmount;
    /** 人流データの最大値 **/
    int PeopleFlowMaxAmount;
    /** ヒートマップの幅 **/
    int HeatmapWidth;
    /** ヒートマップの高さ(奥行) **/
    int HeatmapHeight;
};
