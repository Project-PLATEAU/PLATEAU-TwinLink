// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkPeopleFlowVisualizerBase.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/Texture.h"
#include "Camera/PlayerCameraManager.h"
#include "ConvexVolume.h"

#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUGeometry.h"

#include "TwinLinkCommon.h"
#include "TwinLinkSpatialID.h"
#include "TwinLinkPeopleFlowSystem.h"

#include "TwinLinkWorldViewer.h"


namespace {
    ///** cm2をkm2に変換する **/
    //const auto CM2ToKM2 = 1000000.0;

    // デバッグ描画生存時間
    const float DRAW_DEBUG_LIFE_TIME = 0.1f;

    /**
     * @brief 地形オブジェクトを探す
     * @param CityModel
     * @return
    */
    const UPLATEAUCityObjectGroup* FindDemCityObjectGroup(const APLATEAUInstancedCityModel& CityModel) {
        const UPLATEAUCityObjectGroup* DemCityObj = nullptr;
        TArray<UPLATEAUCityObjectGroup*> CityObjGroups;
        CityModel.GetComponents<UPLATEAUCityObjectGroup>(CityObjGroups, true);
        for (const auto& CityObjGroup : CityObjGroups) {
            const auto bIsGround = CityObjGroup->GetName().StartsWith(TEXT("DEM_"), ESearchCase::IgnoreCase);
            if (bIsGround) {
                DemCityObj = CityObjGroup;
                break;
            }
        }

        check(DemCityObj);
        return DemCityObj;
    }

    /**
     * @brief 高度を取得する
     * @param CityModel
     * @return
    */
    double GetAltitudeFromCityObjGroups(const APLATEAUInstancedCityModel& CityModel) {
        double Altitude = INFINITY;
        const auto DemCityObj = FindDemCityObjectGroup(CityModel);
        Altitude = DemCityObj->GetNavigationBounds().GetCenter().Z;
        check(Altitude != INFINITY);
        return Altitude;
    }

    /**
     * @brief プレイヤー視点の投影行列を取得する
     * @param Self
     * @param World
     * @param OutProjectionData
    */
    void GetPlayerProjectionData(UObject* Self, UWorld* World, FSceneViewProjectionData* OutProjectionData) {
        const auto ViewportClient = World->GetGameViewport();
        APlayerCameraManager* CameraManager =
            World->GetFirstPlayerController()->PlayerCameraManager;
        const auto LocalPlayer =
            UGameplayStatics::GetPlayerController(Self, 0)->GetLocalPlayer();
        LocalPlayer->GetProjectionData(ViewportClient->Viewport, *OutProjectionData);
    }

    /**
     * @brief 視錘台のバウンディングボックスを求める
     * @param OutResult
     * @param EyeViewPorjMatrix
    */
    void CalculateViewFrustumBounds(FConvexVolume* OutResult, const FMatrix& EyeViewPorjMatrix) {
        bool bUseNearPlane = false;
        GetViewFrustumBounds(*OutResult, EyeViewPorjMatrix, bUseNearPlane);
    }

    /**
     * @brief サンプリングを開始する座標を求める
     * @param GeoReference
     * @param RequestStartPosition UE上のワールド座標　これを空間IDに変換して開始地点を求める
     * @param Zoom
     * @return
    */
    FVector CalculateSamplingStartPosition(
        FPLATEAUGeoReference* const GeoReference,
        FVector RequestStartPosition,
        int Zoom) {
        const auto SamplingEdgeSpatialID = FTwinLinkSpatialID::Create(*GeoReference, RequestStartPosition, Zoom, false);
        const auto SamplingEdge = SamplingEdgeSpatialID.GetSpatialIDArea(*GeoReference);
        const auto OffsetToEdge = SamplingEdge.GetCenter();
        return OffsetToEdge;
    }

    /**
     * @brief ボクセルのサイズを計算する
     * @param GeoReference
     * @param Position UE上のワールド座標　空間ID生成時に利用する
     * @param Zoom
     * @return
    */
    FVector CalculateVoxelSize(FPLATEAUGeoReference* const GeoReference,
        FVector Position,
        int Zoom) {
        // CityModelの端っこの空間IDを作成してボクセルサンプル用に使用する
        const auto SpatialIDForSamplingWidth = FTwinLinkSpatialID::Create(*GeoReference, Position, Zoom, false);      // サンプリングの幅を決定するための空間ID。緯度経度によって空間IDのボクセルの大きさが違うので差が小さくなる中心の空間IDを利用する
        const auto SamplingVoxel = SpatialIDForSamplingWidth.GetSpatialIDArea(*GeoReference);

        // ボクセルを平面上に配置する際の情報を計算
        const auto VoxelSize = SamplingVoxel.GetExtent() * 2.0;
        return VoxelSize;
    }

    /**
     * @brief テストデータの生成
     * @param Req
     * @return
    */
    FTWinLinkPeopleFlowApiResult TestCreateTestData(const TArray<FTwinLinkSpatialID> Req, int InMaxZoomLevel) {
        FTWinLinkPeopleFlowApiResult Result;
        for (const auto ReqSpatialID : Req) {
            const auto ZoomLevelDiff = InMaxZoomLevel - ReqSpatialID.GetZ();
            const auto PeopleFlow =
                (ReqSpatialID.GetX() + ReqSpatialID.GetY()) % 8 * 
                ((int)(6420 * FMath::Pow(4.0, ZoomLevelDiff)) / 8);

            Result.Populations.Add(FTwinLinkPopulationData{
                ReqSpatialID, TEXT("Type"),
                TArray<FTwinLinkPopulationValue>{FTwinLinkPopulationValue{FDateTime(), TEXT("Unit"), PeopleFlow}} });
        }
        Result.bSuccess = true;
        return Result;
    }
}

// Sets default values for this component's properties
UTwinLinkPeopleFlowVisualizerBase::UTwinLinkPeopleFlowVisualizerBase() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
    SetThresholdForSubdivision(100000.0);
    bIsRequestActiveState = bIsCurrentActiveState = true;
    bIsWaitingRequestResult = false;

    bIsInitedMaxMinPopulation = false;

    // 東京都の市区町村別面積・人口・人口密度【2023年8月】
    // https://tisikijikan.com/tokyoto-area-population-matome/
    // 6425 人/km2
    MaxPopulation = 6425;                            // 数値は仮
    MinPopulation = MaxPopulation * 0.01;     // 最大値の1％を最小値とする
}


// Called when the game starts
void UTwinLinkPeopleFlowVisualizerBase::BeginPlay() {
    Super::BeginPlay();

    PeopleFlowApi = NewObject<UTwinLinkPeopleFlowApi>();
    // ...

    UpdatePeopleFlowDelegate.BindUFunction(this, FName("OnUpdatePeopleFlow"));
    OnSetMaximumAndMinimumAutomaticallyDelegate.BindUFunction(this, FName("OnSetMaximumAndMinimumAutomatically"));


    TWeakObjectPtr< ATwinLinkWorldViewer> WorldViewer = ATwinLinkWorldViewer::GetInstance(GetWorld());
    if (WorldViewer.IsValid()) {
        WorldViewer->EvOnUpdatedLocationAndRotation.AddLambda([this]() {
            if (IsInited() == false)
                return;
            RequestInfoFromSpatialID();
            });
    }
    else {
        UE_TWINLINK_C_LOG(LogTemp, Log, TEXT("Faild get ATwinLinkWorldViewer."));
    }
}

// Called every frame
void UTwinLinkPeopleFlowVisualizerBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UTwinLinkPeopleFlowVisualizerBase::SetThresholdForSubdivision(double Distance) {
    // 以下の条件の時、ズームレベルを上げて細分化する     細分化するかの閾値
    // 距離 < CriteriaForIncreasingResolution / 2**ZOOM
    //const auto BaseSentimeter = 100000.0;
    ThresholdForSubdivision = Distance * FMath::Pow(2.0, BaseZoomLevel); // 値は仮
}

void UTwinLinkPeopleFlowVisualizerBase::RequestChangeActiveState(bool bIsActiveHeatmap) {
    bIsRequestActiveState = bIsActiveHeatmap;

    UpdateActiveState();
}

void UTwinLinkPeopleFlowVisualizerBase::InitVisualizer(APLATEAUInstancedCityModel* InCityModel, int InBaseZoomLevel, int InMaxZoomLevel) {
    Cache.Clear();

    check(InCityModel);
    // CityModelHelperの初期化
    CityModelHelper.Init(InCityModel);

    BaseZoomLevel = InBaseZoomLevel;
    MaxZoomLevel = InMaxZoomLevel;

    // 時間の設定
    CurrentVisualizeTime = std::nullopt;

    // ヒートマップのエリアを算出
    FVector Center = CityModelHelper.DemCenter;
    FVector Extent = CityModelHelper.DemExtent;
    FPLATEAUGeoReference* GeoReference = CityModelHelper.GeoReference;
    double Altitude = CityModelHelper.Altitude;

    // 高さのある建物にもヒートマップを描画したいので高さの値のみ別で設定
    Extent.Z = CityModelHelper.CityExtent.Z +
        FMath::Abs(Center.Z - CityModelHelper.CityCenter.Z);  // 中心位置の差を加えることで街全体を包める

    const auto SamplingVoxel = FTwinLinkSpatialID::Create(*GeoReference, Center, BaseZoomLevel);

    DrawDebugBox(GetWorld(), Center, Extent, FColor::Red, true);
    // 計算結果によっては端っこの値が入らないことがあるので少し大きめにサイズを取得する
    Extent = Extent + SamplingVoxel.GetSpatialIDArea(*GeoReference).GetExtent();
    Center = Center + SamplingVoxel.GetSpatialIDArea(*GeoReference).GetExtent();

    // 空間IDボクセルヘルパーの初期化
    VoxelHelper.Init(GeoReference, BaseZoomLevel, MaxZoomLevel, Center, Extent, Altitude);

    // ヒートマップの設定

    // ヒートマップに必 要な解像度を求める
    HeatmapWidth = VoxelHelper.SpatialIDXRange;
    HeatmapDepth = VoxelHelper.SpatialIDYRange;   

    // 
    const FVector VoxelBaseCenter =
        VoxelHelper.SamplingVoxelOnBaseLevel.GetCenter();

    HeatmapCenter = VoxelBaseCenter;

    HeatmapExtent = FVector(
        HeatmapWidth * VoxelHelper.SamplingVoxelOnMaxLevel.GetExtent().X,
        HeatmapDepth * VoxelHelper.SamplingVoxelOnMaxLevel.GetExtent().Y,
        Extent.Z);

}


bool UTwinLinkPeopleFlowVisualizerBase::IsInited() const {
    return CityModelHelper.IsValid();
}

void UTwinLinkPeopleFlowVisualizerBase::SetMaximumAndMinimumAutomatically() {
    check(CityModelHelper.IsValid());

    // リクエストの結果待ち
    if (bIsWaitingRequestResult) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Waiting for the result of the request"));
        return;
    }
    bIsWaitingRequestResult = true;

    // サンプリングする際の範囲の端っこを計算
    FPLATEAUGeoReference* GeoReference = CityModelHelper.GeoReference;
    const auto AreaCenter = HeatmapCenter;
    const auto AreaExtent = HeatmapExtent * 2.0;
    const auto Altitude = CityModelHelper.Altitude;
    const auto RequestStartPosition = AreaCenter - FVector(AreaExtent.X, AreaExtent.Y, 0.0) * 0.5;
    const auto Zoom = BaseZoomLevel;
    const auto OffsetToEdge =
        CalculateSamplingStartPosition(GeoReference, RequestStartPosition, Zoom);

    // サンプリングの範囲の端っこデバッグ描画
    DrawDebugPoint(GetWorld(), FVector(OffsetToEdge.X, OffsetToEdge.Y, Altitude), 5.0f, FColor::Red, false, DRAW_DEBUG_LIFE_TIME, UINT8_MAX);

    // CityModelの中心の空間IDを元にボクセルのサイズを求める
    const auto VoxelSize = CalculateVoxelSize(GeoReference, AreaCenter, Zoom);

    // ボクセルのサイズを単位の基準とした時の幅、高さ
    const auto Width = (int)(AreaExtent.X * 2 / VoxelSize.X);
    const auto Depth = (int)(AreaExtent.Y * 2 / VoxelSize.Y);

    // 要求する空間IDの情報
    TArray<FTwinLinkSpatialID> RequestIDs; RequestIDs.Reserve(Width * Depth);
    for (int y = 0; y < Depth; y++) {
        for (int x = 0; x < Width; x++) {
            const auto Orgin = FVector(
                OffsetToEdge.X + x * VoxelSize.X,
                OffsetToEdge.Y + y * VoxelSize.Y,
                Altitude);
            RequestIDs.Add(FTwinLinkSpatialID::Create(*GeoReference, Orgin, Zoom));
        }
    }

    // データを要求する
    //...

    PeopleFlowApi->OnReceivedPeopleFlowResponse.AddUnique(OnSetMaximumAndMinimumAutomaticallyDelegate);

    FTwinLinkPeopleFlowApiRequest Req{ RequestIDs, CurrentVisualizeTime.value_or(FDateTime()), CurrentVisualizeTime.has_value()};
    PeopleFlowApi->Request(Req);
    //OnSetMaximumAndMinimumAutomatically(TestCreateTestData(Req.SpatialIds, MaxZoomLevel));
}

void UTwinLinkPeopleFlowVisualizerBase::SetVisualizeTime(const FDateTime& Time) {
    CurrentVisualizeTime = Time;
}

void UTwinLinkPeopleFlowVisualizerBase::SetVisualizeRealTime() {
    CurrentVisualizeTime = std::nullopt;
}

void UTwinLinkPeopleFlowVisualizerBase::OnSetMaximumAndMinimumAutomatically(const FTWinLinkPeopleFlowApiResult& DataArray) {
    check(bIsWaitingRequestResult);

    if (DataArray.bSuccess == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Request failed : UTwinLinkPeopleFlowVisualizerBase"));
        // リクエスト結果を待たない
        PeopleFlowApi->OnReceivedPeopleFlowResponse.Remove(OnSetMaximumAndMinimumAutomaticallyDelegate);
        bIsWaitingRequestResult = false;

        return;
    }

    int Max = INT32_MIN;
    int Min = INT32_MAX;
    for (const auto Data : DataArray.Populations) {
        if (Data.Values.Num() != 1) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Empty or Unsupported data : UTwinLinkPeopleFlowVisualizerBase"));
            continue;
        }
        const auto PopulationVal = Data.Values[0];
        Max = FMath::Max(PopulationVal.PeopleFlow, Max);
        Min = FMath::Min(PopulationVal.PeopleFlow, Min);
    }

    const auto VoxelVolume = 
        VoxelHelper.SamplingVoxelOnBaseLevel.GetExtent().X * VoxelHelper.SamplingVoxelOnBaseLevel.GetExtent().Y;    // Zは無効値なので
    MaxPopulation = Max;
    MinPopulation = Min;

    // 最大値が最小値以下の時 最大値を最小値よりも大きくする(システム動作のため)
    if (MaxPopulation <= MinPopulation)
        MaxPopulation = MinPopulation + 1.0;

    UE_TWINLINK_LOG(LogTemp, Log, TEXT("MaxPopulation %f"), MaxPopulation);
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("MinPopulation %f"), MinPopulation);

    PeopleFlowApi->OnReceivedPeopleFlowResponse.Remove(OnSetMaximumAndMinimumAutomaticallyDelegate);

    // 初期化済みフラグを立てる
    bIsInitedMaxMinPopulation = true;

    bIsWaitingRequestResult = false;
}

void UTwinLinkPeopleFlowVisualizerBase::RequestInfoFromSpatialID() {
    // 最大値、最小値が計算されていなかったら先に最大値、最小値の計算を行う
    if (bIsInitedMaxMinPopulation == false) {
        SetMaximumAndMinimumAutomatically();
        return;
    }

    check(CityModelHelper.IsValid());

    // リクエストの結果待ち
    if (bIsWaitingRequestResult) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Waiting for the result of the request"));
        return;
    }

    if (bIsCurrentActiveState == false) {
        return;
    }

    //
    const auto GeoReference = CityModelHelper.GeoReference;
    // 地面の高さを取得
    double Altitude = CityModelHelper.Altitude;

    // リクエストする空間IDを求める
    const auto RequestSpatialIDs =
        ExtractRequestSpatialIDs(GeoReference, HeatmapCenter, HeatmapExtent, Altitude, ThresholdForSubdivision);

    // リクエストを行わない
    if (RequestSpatialIDs.IsEmpty())
        return;

    // 空間データのリクエスト
    //...　空間IDの配列と時間を渡す

    bIsWaitingRequestResult = true;

    PeopleFlowApi->OnReceivedPeopleFlowResponse.AddUnique(UpdatePeopleFlowDelegate);

    FTwinLinkPeopleFlowApiRequest Req{ RequestSpatialIDs, CurrentVisualizeTime.value_or(FDateTime()), CurrentVisualizeTime.has_value() };
    PeopleFlowApi->Request(Req);
    //OnUpdatePeopleFlow(TestCreateTestData(Req.SpatialIds, MaxZoomLevel));
}

void UTwinLinkPeopleFlowVisualizerBase::OnUpdatePeopleFlow(const FTWinLinkPeopleFlowApiResult& Result) {
    if (bIsWaitingRequestResult == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Not waiting for the result of the request"));
        return;
    }

    if (Result.bSuccess == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Request failed : UTwinLinkPeopleFlowVisualizerBase"));
        // リクエスト結果を待たない
        PeopleFlowApi->OnReceivedPeopleFlowResponse.Remove(UpdatePeopleFlowDelegate);
        bIsWaitingRequestResult = false;
        return;
    }

    // すべて最大ズームレベルのデータ情報
    Cache.PrimDataAry.Reset();
    TArray<PrimPopulationData>& PrimDataAry = Cache.PrimDataAry;
    PrimDataAry.Reserve(Result.Populations.Num() * FMath::Pow(4.0, MaxZoomLevel - BaseZoomLevel));

    const auto& PopulationAry = Result.Populations;
    for (const auto& Data : PopulationAry) {
        if (Data.Values.IsEmpty()) {
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Empty data : UTwinLinkPeopleFlowVisualizerBase"));
            continue;
        }

        const auto SpatialID = Data.SpatialId;
        check(Data.Values.Num() == 1);
        const auto PopulationVal = Data.Values[0];
        if (SpatialID.GetZ() == MaxZoomLevel) {
            PrimDataAry.Add(PrimPopulationData{ SpatialID.GetX(), SpatialID.GetY(), PopulationVal.PeopleFlow });
            continue;
        }

        int ZoomDiff = MaxZoomLevel - SpatialID.GetZ();
        const auto Len = FMath::Pow(2.0, ZoomDiff);
        for (int Y = 0; Y < Len; Y++) {
            for (int X = 0; X < Len; X++) {
                //const auto& NewPopulationVal = FTwinLinkPopulationValue{
                //PopulationVal.TimeStamp, PopulationVal.Unit,
                //(int)(PopulationVal.PeopleFlow / FMath::Pow(4.0, ZoomDiff)) };  // 人口が少なすぎると0になる  ZoomDiffが5の時 1024

                //PrimDataAry.Add(PrimPopulationData{
                //    (int)(SpatialID.GetX() * FMath::Pow(2.0, ZoomDiff)) + X,
                //    (int)(SpatialID.GetY() * FMath::Pow(2.0, ZoomDiff)) + Y,
                //    NewPopulationVal.PeopleFlow });

                const auto& NewPopulationVal = FTwinLinkPopulationValue{
                    PopulationVal.TimeStamp, PopulationVal.Unit,
                    (int)(PopulationVal.PeopleFlow) };

                PrimDataAry.Add(PrimPopulationData{
                    (int)(SpatialID.GetX() * FMath::Pow(2.0, ZoomDiff)) + X,
                    (int)(SpatialID.GetY() * FMath::Pow(2.0, ZoomDiff)) + Y,
                    NewPopulationVal.PeopleFlow });
            }
        }
    }

    if (PrimDataAry.IsEmpty())
        return;

    // 2*2未満のヒートマップには対応していない
    if (HeatmapWidth < 2 || HeatmapDepth < 2)
        return;

    // 量
    Cache.PopulationArray.Reset();
    TArray<double>& PopulationArray = Cache.PopulationArray;
    PopulationArray.Reserve(PrimDataAry.Num());
    PopulationArray.AddZeroed(HeatmapWidth * HeatmapDepth);

    // データ配列の生成
    for (const auto& Data : PrimDataAry) {
        const auto X = Data.X - (VoxelHelper.SpatialIDXMin);
        const auto Y = Data.Y - (VoxelHelper.SpatialIDYMin);
        const auto Index = X + Y * HeatmapWidth;
        if (Index >= PopulationArray.Num() || Index < 0) {
            continue;
        }

        PopulationArray[Index] =
            FMath::Max(Data.PeopleFlow, DBL_MIN); // データが0じゃない時は小さい値を入れる
        if (Data.PeopleFlow == 0)
            PopulationArray[Index] = 0.0;
    }

    // ヒートマップテクスチャを作成する
    UTexture* Tex = CreateHeatmapTextureByAmount(HeatmapWidth, HeatmapDepth, MinPopulation, MaxPopulation, PopulationArray);

    // エミッシブの強化値を取得する
    const auto EmissiveBoost = TwinLinkGraphicsEnv::GetEmissiveBoostFromEnv(GetWorld());

    // デカールはY軸で-90度回転している X,Z軸の方向が入れ替わる　スケールなので正負の方向は関係ない
    const auto DecalScale = FVector(HeatmapExtent.Z, HeatmapExtent.Y, HeatmapExtent.X);
    const auto Extent = VoxelHelper.SamplingVoxelOnBaseLevel.GetExtent();
    const auto DecalCenter = HeatmapCenter;   // ボクセルの端っこ基準であるため
    UpdateDecalInstance(DecalCenter, HeatmapExtent, Tex, HeatmapWidth, HeatmapDepth, EmissiveBoost);

    // 
    UpdateActiveState();

    PeopleFlowApi->OnReceivedPeopleFlowResponse.Remove(UpdatePeopleFlowDelegate);

    // リクエスト結果を待たない
    bIsWaitingRequestResult = false;

}

void UTwinLinkPeopleFlowVisualizerBase::UpdateActiveState() {
    // 要求されているアクティブ状態に切り替える
    if (bIsRequestActiveState != bIsCurrentActiveState) {
        bIsCurrentActiveState = bIsRequestActiveState;
        ChangeActiveState(bIsCurrentActiveState);
    }
}

/**
 * @brief 必要な空間IDを取得する
 * @param Self
 * @param CityModel
 * @return
*/
TArray<FTwinLinkSpatialID> UTwinLinkPeopleFlowVisualizerBase::ExtractRequestSpatialIDs(
    FPLATEAUGeoReference* const GeoReference, const FVector& AreaCenter, const FVector& AreaExtent, double Altitude,
    double InThresholdForSubdivision) {
    const auto bIsEnableDrawDebug = false;

    // ワールド取得
    UWorld* World = GetWorld();
    check(World);

    // 視点の投影行列を計算
    FSceneViewProjectionData ProjectionData;
    GetPlayerProjectionData(this, World, &ProjectionData);
    const auto EyeViewPorjMatrix = ProjectionData.ComputeViewProjectionMatrix();

    //CityModelのBoundsのテスト描画
    if (bIsEnableDrawDebug)
        DrawDebugBox(World, AreaCenter, AreaExtent, FColor::Cyan, false, DRAW_DEBUG_LIFE_TIME);

    // カメラ視点の視推台バウンドを計算
    FConvexVolume ViewFrustumBounds;
    CalculateViewFrustumBounds(&ViewFrustumBounds, EyeViewPorjMatrix);

    // ズーム値
    const auto Zoom = BaseZoomLevel;

    // 錐台バウンド内に含まれるボクセルを算出   （各ボクセルについて、再帰的に以下を計算←この処理はまだ）
    for (auto& Pair : Cache.LocationsWithZoomMap) {
        Pair.Value.Reset();
    }
    TMap<int, TArray<FVector>>& LocationsWithZoomMap = Cache.LocationsWithZoomMap;

    // VoxelHelperに移植したい
    const auto RequestStartPosition = 
        FTwinLinkSpatialID::Create(
            *GeoReference, VoxelHelper.SamplingEdgeVoxelOnMaxLevel.GetCenter(), 
            BaseZoomLevel, false).GetSpatialIDArea(*GeoReference).GetCenter();

    const auto ZoomLevelFactor = (int)(FMath::Pow(2.0, MaxZoomLevel - BaseZoomLevel));
    const auto Width = HeatmapWidth / ZoomLevelFactor;
    const auto Depth = HeatmapDepth / ZoomLevelFactor;

    // 視推台内のボクセルの座標を抽出
    ExtractVoxelLocationsInView(
        &LocationsWithZoomMap, ThresholdForSubdivision,
        GeoReference, ProjectionData.ViewOrigin, ViewFrustumBounds, Altitude,
        Zoom, MaxZoomLevel, 
        RequestStartPosition,
        Width, Depth,
        AreaCenter, FVector2D(AreaExtent), bIsEnableDrawDebug ? World : nullptr);

    // 空間IDを計算
    Cache.RequestSpatialIDs.Reset();
    TArray<FTwinLinkSpatialID>& RequestSpatialIDs = Cache.RequestSpatialIDs;
    RequestSpatialIDs.Reserve(LocationsWithZoomMap.Num());
    for (const auto LocationWithZoom : LocationsWithZoomMap) {
        const auto Z = LocationWithZoom.Key;
        for (const auto Location : LocationWithZoom.Value) {
            RequestSpatialIDs.Add(FTwinLinkSpatialID::Create(*GeoReference, Location, Z, false));
        }
    }

    // 空間ＩＤの表現域をデバッグ描画
    if (bIsEnableDrawDebug) {
        for (const auto SpatialID : RequestSpatialIDs) {
            const auto Area = SpatialID.GetSpatialIDArea(*GeoReference);
            const auto DebugBoxCenter = FVector(Area.GetCenter().X, Area.GetCenter().Y, Altitude);
            const auto DebugBoxExtent = FVector(Area.GetExtent().X, Area.GetExtent().Y, 5000) * 0.9;
            DrawDebugBox(World, DebugBoxCenter, DebugBoxExtent, FColor::Blue, false, DRAW_DEBUG_LIFE_TIME, UINT8_MAX);
        }
    }

    return RequestSpatialIDs;
}


void UTwinLinkPeopleFlowVisualizerBase::ExtractVoxelLocationsInView(
    TMap<int, TArray<FVector>>* const OutLocationsMap,
    double InThresholdForSubdivision,
    FPLATEAUGeoReference* const GeoReference,
    const FVector& ViewLocation, const FConvexVolume& ViewFrustumBounds, double Altitude,
    double Zoom, int InMaxZoomLevel,
    const FVector& RequestStartPosition,
    int Width, int Depth,
    const FVector& AreaCenter, const FVector2D& AreaExtent,
    const UWorld* World) {

    // サンプリングする際の範囲の端っこを計算
    const auto OffsetToEdge =
        CalculateSamplingStartPosition(GeoReference, RequestStartPosition, Zoom);

    // サンプリングの範囲の端っこデバッグ描画
    if (World)
        DrawDebugPoint(World, FVector(OffsetToEdge.X, OffsetToEdge.Y, Altitude), 5.0f, FColor::Red, false, DRAW_DEBUG_LIFE_TIME, UINT8_MAX);

    // CityModelの中心の空間IDを元にボクセルのサイズを求める
    const auto VoxelSize = CalculateVoxelSize(GeoReference, AreaCenter, Zoom);

    // 錐台バウンド内に含まれるボクセルを算出   （各ボクセルについて、再帰的に以下を計算←この処理はまだ）
    TArray<FVector> ContainLocations; ContainLocations.Reserve(Width * Depth);
    ExtractVoxelLocationsInView(
        &ContainLocations,
        Width, Depth,
        ViewFrustumBounds,
        OffsetToEdge, VoxelSize, Altitude);

    // 細分化必要なボクセルを抽出する　（ただしズームレベルが一定値以上高いと負荷が高いだけなので制限を掛ける）
    TArray<FVector> SubdiviveLocations; SubdiviveLocations.Reserve(ContainLocations.Num());
    if (Zoom < InMaxZoomLevel) {
        for (const auto Location : ContainLocations) {
            const auto DistanceSqr = FVector::DistSquared(Location, ViewLocation);
            const auto CriteriaForSubdivision = ThresholdForSubdivision * FMath::Pow(0.5, Zoom);
            if (DistanceSqr < CriteriaForSubdivision * CriteriaForSubdivision) {
                SubdiviveLocations.Add(Location);
            }
        }
    }

    // 細分化対象のボクセルを削除する
    for (const auto Location : SubdiviveLocations) {
        ContainLocations.Remove(Location);
    }

    // 細分化
    for (const auto Location : SubdiviveLocations) {
        const auto SpatialID = FTwinLinkSpatialID::Create(*GeoReference, Location, Zoom, false);
        const auto SpatialIDVoxel = SpatialID.GetSpatialIDArea(*GeoReference);
        const auto RequestStartPosition = SpatialIDVoxel.GetCenter() - SpatialIDVoxel.GetExtent() * 0.5;

        const auto DivNum = 2;  // Zoomレベルを1上げると １辺２つに分割されるので2
        ExtractVoxelLocationsInView(
            OutLocationsMap, ThresholdForSubdivision, GeoReference, ViewLocation, ViewFrustumBounds, Altitude,
            Zoom + 1, InMaxZoomLevel, 
            RequestStartPosition,
            DivNum, DivNum,
            SpatialIDVoxel.GetCenter(), FVector2D(SpatialIDVoxel.GetExtent()), World);
    }

    // ボクセルのデバッグ描画
    if (World) {
        for (const auto Location : ContainLocations) {
            const auto DebugBoxExtent = FVector(VoxelSize.X * 0.5, VoxelSize.Y * 0.5, 5000) * 0.95;
            const auto DebugBoxCenter = Location;
            DrawDebugBox(World, DebugBoxCenter, DebugBoxExtent, FColor::Emerald, false, DRAW_DEBUG_LIFE_TIME, UINT8_MAX);
        }
    }

    // 結果を格納
    TArray<FVector>& Locations = OutLocationsMap->FindOrAdd(Zoom);
    Locations.Append(ContainLocations);
}

void UTwinLinkPeopleFlowVisualizerBase::ExtractVoxelLocationsInView(TArray<FVector>* const OutLocations, int Width, int Depth, const FConvexVolume& ViewConvexVolume, const FVector& OffsetToEdge, const FVector& VoxelSize, double Altitude) {
    bool bOutFullyContained = false;
    for (int y = 0; y < Depth; y++) {
        for (int x = 0; x < Width; x++) {
            const auto Orgin = FVector(
                OffsetToEdge.X + x * VoxelSize.X,
                OffsetToEdge.Y + y * VoxelSize.Y,
                Altitude);
            const auto bIsContain = ViewConvexVolume.IntersectBox(Orgin, VoxelSize, bOutFullyContained);
            if (bIsContain)
                OutLocations->Add(Orgin);
        }
    }

}

UTexture* UTwinLinkPeopleFlowVisualizerBase::CreateSampleHeatmapTexture() {
    const auto Width = 256;
    const auto Height = 256;

    TArray<float> HeatLevels;
    HeatLevels.Reserve(Width * Height);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            const auto Index = x + y * Width;
            const auto Level = FMath::Clamp((FMath::Sin((x + y) * 0.05f) + 1) * 0.5f * 255, 0, 255);
            HeatLevels.Add(Level);
        }
    }

    return CreateHeatmapTexture(Width, Height, HeatLevels);
}

UTexture* UTwinLinkPeopleFlowVisualizerBase::CreateHeatmapTexture(int Width, int Height, const TArray<float>& HeatLevels) {
    check(HeatLevels.Num() == Width * Height);

    // 利用できるキャッシュテクスチャを取得する
    UTexture2D* CacheTexture = nullptr;
    if (Cache.HeatmapTexture != nullptr) {
        UTexture2D* TempCacheTexture = Cast<UTexture2D>(Cache.HeatmapTexture.Get());
        if (TempCacheTexture->GetSizeX() == Width && TempCacheTexture->GetSizeY() == Height) {
            CacheTexture = TempCacheTexture;
        }
    }

    // 利用するテクスチャを用意する
    UTexture2D* CustomTexture = nullptr;
    if (CacheTexture == nullptr) {
        CustomTexture = UTexture2D::CreateTransient(
            Width, Height, EPixelFormat::PF_R8G8B8A8);
        CustomTexture->Filter = TextureFilter::TF_Nearest;
        Cache.HeatmapTexture = CustomTexture;
    }
    else {
        CustomTexture = CacheTexture;
    }

    // テクスチャへの書き込み
    FTexture2DMipMap* MipMap = &CustomTexture->GetPlatformData()->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint32* RawImageData = (uint32*)ImageData->Lock(LOCK_READ_WRITE);
    FMemory::Memzero(RawImageData, Width * Height);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            const auto Index = x + y * Width;
            const auto Level = FMath::Clamp(HeatLevels[Index], 0.0f, 1.0f);
            const auto Color = FColor(Level * 255, Level * 255, Level * 255);
            //const auto Color = FColor(((x + y) % 4) * 84 + 1);
            RawImageData[Index] = Color.DWColor();
        }
    }

    ImageData->Unlock();
    CustomTexture->UpdateResource();

    return CustomTexture;
}

UTexture* UTwinLinkPeopleFlowVisualizerBase::CreateHeatmapTextureByAmount(int Width, int Height, double MinAmount, double MaxAmount, const TArray<double>& Amounts) {
    check(MaxAmount > MinAmount);

    Cache.HeatLevels.Reset();
    TArray<float>& HeatLevels = Cache.HeatLevels;
    HeatLevels.Reserve(Width * Height);
    const auto DiffMaxBetweenMin = MaxAmount - MinAmount;
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {

            const auto Index = x + y * Width;
            // MinAmountToAmount >= 0 && MinAmountToAmount <= DiffMaxBetweenMin
            const auto MinAmountToAmount =
                FMath::Min(Amounts[Index] - MinAmount, DiffMaxBetweenMin);    // 0<MinAmountToAmount<=DiffMaxBetweenMin

            double Level =
                FMath::Clamp(MinAmountToAmount / DiffMaxBetweenMin, (1.0 / 255.0), 1.0); // 0<Level<=1  正しAmounts[Index]が0の時は0になる
            if (Amounts[Index] == 0)
                Level = 0.0;
            HeatLevels.Add(Level);
        }
    }

    return CreateHeatmapTexture(Width, Height, HeatLevels);
}

void UTwinLinkPeopleFlowVisualizerBase::FCityModelHelper::Init(APLATEAUInstancedCityModel* InCityModel) {
    check(InCityModel);
    CityModel = InCityModel;

    //
    GeoReference = &CityModel->GeoReference;

    // 地形のメッシュを取得
    const auto DemCityObj = FindDemCityObjectGroup(*CityModel);

    // 地形のエリアを算出
    DemCenter = DemCityObj->GetNavigationBounds().GetCenter();
    DemExtent = DemCityObj->GetNavigationBounds().GetExtent();

    Altitude = DemCityObj->GetNavigationBounds().GetExtent().Z;
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("Altitude %f"), Altitude);

    // 高さのある建物にもヒートマップを描画したいので高さの値のみ別で設定
    CityModel->GetActorBounds(false, CityCenter, CityExtent, true);

}

void UTwinLinkPeopleFlowVisualizerBase::FVoxelHelper::Init(
    FPLATEAUGeoReference* GeoReference, int InBaseZoomLevel, int InMaxZoomLevel, const FVector& Center, const FVector& Extent, double Altitude) {

    // 仮の空間IDBox 値を調整後　空間IDBoxを改めて作成する
    const auto TempSamplingEdge =
        FTwinLinkSpatialID::Create(*GeoReference, Center - Extent, InBaseZoomLevel, false);

    FTwinLinkSpatialID TempSamplingEdgeEnd =
        FTwinLinkSpatialID::Create(*GeoReference, Center + Extent, InBaseZoomLevel, false);

    const auto XDiff = TempSamplingEdgeEnd.GetX() - (TempSamplingEdge.GetX() - 1);
    const auto YDiff = TempSamplingEdgeEnd.GetY() - (TempSamplingEdge.GetY() - 1);


    // 偶数であるときに奇数になるように範囲と中心地点を調整する
    // 奇数にしている理由　中心のボクセルを存在させるため
    const auto bIsEvenX = XDiff % 2 == 0;
    const auto bIsEvenY = YDiff % 2 == 0;

    TempSamplingEdgeEnd = FTwinLinkSpatialID::Create(
        TempSamplingEdgeEnd.GetZ(),
        TempSamplingEdgeEnd.GetF(),
        TempSamplingEdgeEnd.GetX() + (bIsEvenX ? 1 : 0),
        TempSamplingEdgeEnd.GetY() + (bIsEvenY ? 1 : 0));


    const auto TempSamplingEdgeBox = TempSamplingEdge.GetSpatialIDArea(*GeoReference);
    const auto TempSamplingEdgeEndBox = TempSamplingEdgeEnd.GetSpatialIDArea(*GeoReference);
    const auto EdgePos = (TempSamplingEdgeBox.GetCenter() - TempSamplingEdgeBox.GetExtent());
    const auto EdgeEndPos = (TempSamplingEdgeEndBox.GetCenter() + TempSamplingEdgeEndBox.GetExtent());
    FVector CustomExtent = (EdgeEndPos - EdgePos) * 0.5; // 境界線も含めたいので ボクセルの半径よりも小さい値を追加
    FVector CustomCenter = (EdgePos + EdgeEndPos) * 0.5;

    const auto SpatialIDCenterBaseLevel =
        FTwinLinkSpatialID::Create(*GeoReference, CustomCenter, InBaseZoomLevel, false);
    SamplingVoxelOnBaseLevel = SpatialIDCenterBaseLevel.GetSpatialIDArea(*GeoReference);

    const auto SpatialIDCenterMaxLevel =
        FTwinLinkSpatialID::Create(*GeoReference, CustomCenter, InMaxZoomLevel, false);
    SamplingVoxelOnMaxLevel = SpatialIDCenterMaxLevel.GetSpatialIDArea(*GeoReference);


    const auto ZoomDiff = InMaxZoomLevel - InBaseZoomLevel;
    {
        const auto SamplingEdgeSpatialID = TempSamplingEdge.ZoomUp(ZoomDiff, 0, 0);
        SpatialIDXMin = SamplingEdgeSpatialID.GetX();
        SpatialIDYMin = SamplingEdgeSpatialID.GetY();

        SamplingEdgeVoxelOnMaxLevel = SamplingEdgeSpatialID.GetSpatialIDArea(*GeoReference);
    }

    {
        const auto Offset = FMath::Pow(2.0, ZoomDiff) - 1;
        const auto SamplingEndEdgeSpatialID = TempSamplingEdgeEnd.ZoomUp(ZoomDiff, Offset, Offset);
        const auto SpatialIDXMax = SamplingEndEdgeSpatialID.GetX();
        const auto SpatialIDYMax = SamplingEndEdgeSpatialID.GetY();

        SpatialIDXRange = SpatialIDXMax - (SpatialIDXMin - 1);  // SpatialIDXMinを含むので-1
        SpatialIDYRange = SpatialIDYMax - (SpatialIDYMin - 1);
    }

    // 最小ボクセルの面積を計算
    const auto MinumVoxel = SamplingVoxelOnMaxLevel;
    MinmumVoxelArea = MinumVoxel.GetExtent().X * MinumVoxel.GetExtent().Y;


}

void UTwinLinkPeopleFlowVisualizerBase::FCache::Clear() {
    PrimDataAry.Reset();
    LocationsWithZoomMap.Reset();
    RequestSpatialIDs.Reset();
    PopulationArray.Reset();
    HeatLevels.Reset();
    HeatmapTexture = nullptr;
}