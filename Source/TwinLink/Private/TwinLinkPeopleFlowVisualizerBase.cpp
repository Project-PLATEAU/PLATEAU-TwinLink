// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkPeopleFlowVisualizerBase.h"
#include "TwinLinkCommon.h"
#include "Engine/Texture.h"

#include "TwinLinkSpatialID.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUGeometry.h"

// Sets default values for this component's properties
UTwinLinkPeopleFlowVisualizerBase::UTwinLinkPeopleFlowVisualizerBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTwinLinkPeopleFlowVisualizerBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTwinLinkPeopleFlowVisualizerBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTwinLinkPeopleFlowVisualizerBase::TestFunc()
{
    // 西新宿
    //const auto Lat = 35.694547886155775;
    //const auto Lng = 139.69296552318713;
    // 京王プラザホテル　合ってるぽい
    const auto Lat = 35.690566286523875;
    const auto Lng = 139.69436739188453;
    const auto Zoom = 25.0;
    //const auto Alt = 10.0;
    const auto Alt = 26.0;

    const auto SpatialID =
        FTwinLinkSpatialID::Create(Lat, Lng, Zoom, Alt, true);

    UE_TWINLINK_LOG(LogTemp, Log, TEXT("SpatialID Z:%d F:%d X:%d Y:%d"),
        SpatialID.GetZ(), SpatialID.GetF(), SpatialID.GetX(), SpatialID.GetY());
    //京王プラザホテル TwinLinkLogTemp: SpatialID Z : 25 F : 10 X : 29797674 Y : 13211926

    const auto WorldBaseSpatialID = FTwinLinkSpatialID::Create(
        SpatialID.GetZ(), SpatialID.GetF(),
        SpatialID.GetX(), SpatialID.GetY(), SpatialID.IsValidAltitude());

    UE_TWINLINK_LOG(LogTemp, Log, TEXT("WorldBaseSpatialID Z:%d F:%d X:%d Y:%d"),
        WorldBaseSpatialID.GetZ(), WorldBaseSpatialID.GetF(), 
        WorldBaseSpatialID.GetX(), WorldBaseSpatialID.GetY());
    //京王プラザホテル TwinLinkLogTemp : WorldBaseSpatialID Lat : 139.694359 Lng : 35.690573 Zoom : 25.000000 Alt : 0.000000
    //Altの制度が悪い？
}

FVector UTwinLinkPeopleFlowVisualizerBase::TestFuncProjectSpatialID(APLATEAUInstancedCityModel* CityModel) {
    check(CityModel);

    // 西新宿
    //const auto Lat = 35.694547886155775;
    //const auto Lng = 139.69296552318713;
    // 京王プラザホテル　合ってるぽい
    const auto Lat = 35.690566286523875;
    const auto Lng = 139.69436739188453;
    const auto Zoom = 25.f;
    const auto Alt = 0.f;

    FPLATEAUGeoReference GeoRefrence = CityModel->GeoReference;
    FPLATEAUGeoCoordinate Coordinate;
    Coordinate.Latitude = Lat;
    Coordinate.Longitude = Lng;
    Coordinate.Height = Alt;
    const auto Pos = UPLATEAUGeoReferenceBlueprintLibrary::Project(GeoRefrence, Coordinate);

    UE_TWINLINK_LOG(LogTemp, Log, TEXT("Pos %f,%f,%f"),
        Pos.X, Pos.Y, Pos.Z);
    //TwinLinkLogTemp: Pos 7565.478639,-7655.427662,0.000000

    return Pos;
}

void UTwinLinkPeopleFlowVisualizerBase::TestFuncSpatialID(APLATEAUInstancedCityModel* CityModel) {
    // 京王プラザホテル
    const auto Lat = 35.690566286523875;
    const auto Lng = 139.69436739188453;
    const auto Zoom = 25.0;
    const auto Alt = 0.0;

    FVector UEWorldPos(7565.478639, -7655.427662, 500.000000);


    //　　テストの項目です。１．２はマージ前に確認予定です。
    //    〇UE上のワールド座標から適切な緯度経度に変換できるか　（高度あってる？）
    //    〇空間IDをUE上のワールド座標でボクセルの範囲を表現出来ているか
    //    緯度経度が適切な空間IDに変換されているか


    FPLATEAUGeoReference GeoRefrence = CityModel->GeoReference;
    const auto SpatialID = FTwinLinkSpatialID::Create(GeoRefrence, UEWorldPos, Zoom, true);
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("SpatialID Z:%d F:%d X:%d Y:%d"),
        SpatialID.GetZ(), SpatialID.GetF(), SpatialID.GetX(), SpatialID.GetY());
    // {Latitude = 35.690566286523868 Longitude = 139.69436739188447 Height = 4.9999998882412910 }
    // TwinLinkLogTemp: SpatialID Z : 25 F : 4 X : 29797674 Y : 13211926

    const auto Area = SpatialID.GetSpatialIDArea(GeoRefrence);
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("Area %f,%f,%f     %f,%f,%f"),
        Area.Min.X, Area.Min.Y, Area.Min.Z,
        Area.Max.X, Area.Max.Y, Area.Max.Z);
    // TwinLinkLogTemp : Area 7492.372517, -7731.269986, 450.000010     7589.337730, -7634.460340, 550.000012

}

void UTwinLinkPeopleFlowVisualizerBase::RequestInfoFromSpatialID() {

    // 空間データのリクエスト

}

void UTwinLinkPeopleFlowVisualizerBase::UpdatePeopleFlow(const TArray<TestSpatialData>& DataArray) {
    check(DataArray.IsEmpty() == false);
    check(DataArray.Num() >= 2*2);      // 最低サイズ2*2を想定

    // 空間IDのX,Y最小、最大値
    int XMin = 0;
    int XMax = 0;
    int YMin = 0;
    int YMax = 0;

    // 人流データの最小、最大値
    int MinAmount = INT32_MAX;
    int MaxAmount = 0;

    // 最小、最大値を計算
    for (const auto& Data : DataArray) {
        XMin = FMath::Min(XMin, Data.X);
        XMax = FMath::Max(XMax, Data.X);
        YMin = FMath::Min(YMin, Data.Y);
        YMax = FMath::Max(YMax, Data.Y);

        MinAmount = FMath::Min(MinAmount, Data.PeopleFlow);
        MaxAmount = FMath::Min(MaxAmount, Data.PeopleFlow);
    }

    // 各値の範囲
    const auto XRange = XMax - XMin;
    const auto YRange = YMax - YMin;
    check(XRange == YRange);    // 四角形のみ対応
    const auto AmountRange = MaxAmount - MinAmount;
    check(AmountRange > 0);     // 0除算対策

    // 量
    TArray<int> AmountArray;
    AmountArray.Reserve(DataArray.Num());
    AmountArray.AddZeroed(DataArray.Num());

    for (const auto& Data : DataArray) {
        const auto X = Data.X - XMin;
        const auto Y = Data.Y - YMin;
        const auto Index = X + Y * XRange;
        AmountArray[Index] = Data.PeopleFlow;
    }

    // クラス変数の更新
    PeopleFlowMinAmount = MinAmount;
    PeopleFlowMaxAmount = MaxAmount;
    HeatmapWidth = XRange;
    HeatmapHeight = YRange;

    // ヒートマップテクスチャを作成する
    UTexture* Tex = CreateHeatmapTextureByAmount(XRange, YRange, MinAmount, MaxAmount, AmountArray);

    // ヒートマップのテクスチャを設定する
    //...
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
    check(HeatLevels.Num() == Width * Height)

    UTexture2D* CustomTexture = UTexture2D::CreateTransient(
        Width, Height, EPixelFormat::PF_R8G8B8A8, TEXT("Heatmap"));

    FTexture2DMipMap* MipMap = &CustomTexture->PlatformData->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint32* RawImageData = (uint32*)ImageData->Lock(LOCK_READ_WRITE);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            const auto Index = x + y * Width;
            const auto Level = FMath::Clamp((FMath::Sin((x + y) * 0.05f) + 1) * 0.5f * 255, 0, 255);
            const auto Color = FColor(HeatLevels[Index], HeatLevels[Index], HeatLevels[Index]);
            RawImageData[Index] = Color.DWColor();
        }
    }

    ImageData->Unlock();
    CustomTexture->UpdateResource();

    return CustomTexture;
}

UTexture* UTwinLinkPeopleFlowVisualizerBase::CreateHeatmapTextureByAmount(int Width, int Height, int MinAmount, int MaxAmount, const TArray<int>& Amounts) {
    check(MaxAmount > MinAmount);

    TArray<float> HeatLevels;
    HeatLevels.Reserve(Width * Height);
    
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            const auto Index = x + y * Width;
            const auto DiffMaxBetweenMin = MaxAmount - MinAmount;
            // MinAmountToAmount >= 0 && MinAmountToAmount <= DiffMaxBetweenMin
            const auto MinAmountToAmount = 
                FMath::Clamp(Amounts[Index] - MinAmount, 0, DiffMaxBetweenMin); 
            
            const auto Level = (float)MinAmountToAmount / DiffMaxBetweenMin; // 0-1
            HeatLevels.Add(Level);
        }
    }

    return CreateHeatmapTexture(Width, Height, HeatLevels);
}

