// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Misc/DateTime.h"
#include "TwinLinkPeopleFlowSystem.h"
#include "TwinLinkPeopleFlowVisualizerBase.generated.h"

class FPLATEAUGeoReference;
class APLATEAUInstancedCityModel;
class UTexture;

/**
 * @brief 人流データの可視化機能を提供するコンポーネント
*/
UCLASS(Abstract, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TWINLINK_API UTwinLinkPeopleFlowVisualizerBase : public USceneComponent {
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
    void SetThresholdForSubdivision(double Distance);

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestChangeActiveState(bool bIsActiveHeatmap);

    /**
     * @brief 都市モデルの設定
     * @param CityModel
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void InitVisualizer(APLATEAUInstancedCityModel* InCityModel, int InBaseZoomLevel = 19, int InMaxZoomLevel = 19);

    /**
     * @brief 初期化済みか？
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    bool IsInited() const;

    /**
     * @brief 最大値と最小値を自動で計算する
     * かなり負荷が高い。数フレーム掛かる可能性がある。
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void SetMaximumAndMinimumAutomatically();

    /**
     * @brief 表示する時間帯を指定する
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void SetVisualizeTime(const FDateTime& Time);


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
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
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
     * @brief ヒートマップ用のテクスチャを作成
     * 量は同一面積での値にすること
     * @param Width 幅
     * @param Height 高さ
     * @param MinAmount 量の最小値
     * @param MaxAmount 量の最大値
     * @param Amounts 量が格納された配列
     *  assert(HeatLevels.Num() == Width*Height)
     * @return
    */
    UTexture* CreateHeatmapTextureByAmount(int Width, int Height,
        double MinAmount, double MaxAmount, const TArray<double>& Amounts);

    /**
     * @brief ヒートマップのデカールを更新する
     * @param Center 中心地点
     * @param Scale デカールのサイズ
     * @param HeatmapTexture
     * @param HeatmapResolution ヒートマップの解像度
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void UpdateDecalInstance(
        FVector Center, FVector Scale, UTexture* HeatmapTexture, int InHeatmapWidth, int InHeatmapHeight, float EmissiveBoost);

    /**
     * @brief 有効、無効の状態を切り替える
     * @param bIsActiveState
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void ChangeActiveState(bool bIsActiveState);

private:
    /**
     * @brief CityModel関係のメンバー変数をまとめた構造体
    */
    struct FCityModelHelper {
        FCityModelHelper()
            : CityModel(nullptr) 
        {}

        void Init(APLATEAUInstancedCityModel* InCityModel);
        bool IsValid() const { return CityModel != nullptr; }

        // 以下 直接変更は禁止
        
        // CityModel関係
        APLATEAUInstancedCityModel* CityModel;
        FPLATEAUGeoReference* GeoReference;

        // 地形
        FVector DemCenter;
        FVector DemExtent;
        double Altitude;

        // 街
        FVector CityCenter;
        FVector CityExtent;
    };

    /**
     * @brief 空間IDとボクセル周りのメンバー変数をまとめた構造体
    */
    struct FVoxelHelper {
        FVoxelHelper()
        {}

        // 以下 直接変更は禁止

        void Init(
            FPLATEAUGeoReference* GeoReference, 
            int InBaseZoomLevel, int InMaxZoomLevel,
            const FVector& Center,
            const FVector& Extent,
            double Altitude);

        /** 最大ズームレベルでの最小空間IDのX**/
        int SpatialIDXMin;
        /** 最大ズームレベルでの最小空間IDのY**/
        int SpatialIDYMin;

        /** 最大ズームレベルでの最小空間IDを基点とした範囲のX**/
        int SpatialIDXRange;
        /** 最大ズームレベルでの最大空間IDのY**/
        int SpatialIDYRange;

        /** 最小ボクセルの面積 **/
        double MinmumVoxelArea;

        /** サンプリング用の空間IDのボクセル 中心座標 ベースレベル **/
        FBox SamplingVoxelOnBaseLevel;

        /** サンプリング用の空間IDのボクセル 中心座標 最大ズームレベル **/
        FBox SamplingVoxelOnMaxLevel;

        /** サンプリング用の空間IDのボクセル 中心座標 最大ズームレベル **/
        FBox SamplingEdgeVoxelOnMaxLevel;

    };

    /**
     * @brief データ処理で扱う最低限のサイズの構造体
     * memo リクエストしたデータをそのまま扱うとページング出来るサイズを超えたため
    */
    struct PrimPopulationData {
        int X;
        int Y;
        int PeopleFlow;
    };

    struct FCache {
        // キャッシュを削除
        void Clear();

        // プリミティブデータ用　サーバーから情報を受け取って適切な形に変換した結果を格納する配列
        TArray<PrimPopulationData> PrimDataAry;

        // ズーム値と紐づく座標群を保持した配列　視野内の空間IDを取ってくるところで使用
        TMap<int, TArray<FVector>> LocationsWithZoomMap;

        // リクエストする空間IDの配列
        TArray<FTwinLinkSpatialID> RequestSpatialIDs;

        // 密度を格納した配列　ヒートマップ作成に使う
        TArray<double> PopulationDensityArray;

        // ヒートレベル用
        TArray<float> HeatLevels;

        // ヒートマップのテクスチャ
        UPROPERTY()
        TObjectPtr<UTexture> HeatmapTexture;
    };

public:
    /**
     * @brief 人流データの更新を行う
     * コールバック用
     * @param DataArray
    */
    UFUNCTION(Category = "TwinLink")
    void OnUpdatePeopleFlow(const FTWinLinkPeopleFlowApiResult& DataArray);
    /**
     * @brief 最大値と最長値を自動で計算する
     * コールバック用
     * @param DataArray 
    */
    UFUNCTION(Category = "TwinLink")
    void OnSetMaximumAndMinimumAutomatically(const FTWinLinkPeopleFlowApiResult& DataArray);
    
private:
    /**
     * @brief アクティブ状態を更新する
     * 非アクティブだとデカールが非表示になったりヒートマップの更新が行われない
    */
    void UpdateActiveState();

    /**
     * @brief 必要な空間IDを取得する
     * @param GeoReference
     * @param AreaCenter 
     * @param AreaExtent 
     * @param Altitude 
     * @param ThresholdForSubdivision 
     * @return 
    */
    TArray<FTwinLinkSpatialID> ExtractRequestSpatialIDs(
        FPLATEAUGeoReference* const GeoReference,
        const FVector& AreaCenter, const FVector& AreaExtent, double Altitude,
        double InThresholdForSubdivision);

    void ExtractVoxelLocationsInView(
        TMap<int, TArray<FVector>>* const OutLocationsMap, double InThresholdForSubdivision, FPLATEAUGeoReference* const GeoReference,
        const FVector& ViewLocation, const FConvexVolume& ViewFrustumBounds, double Altitude,
        double Zoom, int InMaxZoomLevel,
        const FVector& RequestStartPosition,
        int Width, int Depth,
        const FVector& AreaCenter, const FVector2D& AreaExtent,
        const UWorld* World = nullptr);

    void ExtractVoxelLocationsInView(
        TArray<FVector>* const OutLocations,
        int Width, int Depth,
        const FConvexVolume& ViewConvexVolume,
        const FVector& OffsetToEdge,
        const FVector& VoxelSize,
        double Altitude);

private:
    /** 都市モデルデータ関係の補助構造体 **/
    FCityModelHelper CityModelHelper;
    /** 空間IDボクセル関係の補助構造体 **/
    FVoxelHelper VoxelHelper;
    /** 表示中の時間 **/
    FDateTime CurrentVisualizeTime;

    // 基準のズームレベル 20~24 (サーバーが負荷に耐えられないので一時的に17-19を推奨)
    int BaseZoomLevel;        // BASE_ZOOM_LEVEL < LIMIT_MAX_ZOOM_LEVEL
    // 最大ズームレベル　ズームレベル可変の処理に制限を掛ける際の値
    int MaxZoomLevel;   // 1上がるごとに2倍の解像度　　2^LIMIT_MAX_ZOOM_LEVEL 倍   緯度経度によるが35だと空間IDの表現幅は大体1cm以下

    /** 機能の無効、有効を切り替える要求の状態 **/
    bool bIsRequestActiveState;
    /** 機能が有効化どうか **/
    bool bIsCurrentActiveState;

    /** 空間情報のリクエストの待機状態 **/
    bool bIsWaitingRequestResult;

    /** ヒートマップの中心座標 **/
    FVector HeatmapCenter;
    /** ヒートマップの大きさ **/
    FVector HeatmapExtent;
    /** ヒートマップの幅(画素単位) **/
    int HeatmapWidth;
    /** ヒートマップの高さ(画素単位)(奥行) **/
    int HeatmapDepth;

    /** 人流データの密度の最大値(人/m2) **/
    double MaxPopulationDensity;
    /** 人流データの密度の最小値 **/
    double MinPopulationDensity;

    /** ズームレベルを引き上げるかの閾値 **/
    double ThresholdForSubdivision;
    
    /** 人流データの更新デリゲート **/
    FScriptDelegate UpdatePeopleFlowDelegate;
    /** 最大値、最小値を自動で計算する機能のデリゲート **/
    FScriptDelegate OnSetMaximumAndMinimumAutomaticallyDelegate;

    /** キャッシュデータ **/
    FCache Cache;
};
