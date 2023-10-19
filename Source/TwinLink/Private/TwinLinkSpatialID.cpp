// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkSpatialID.h"

#include <algorithm>

#include "TwinLinkCommon.h"

#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUGeometry.h"
#include "TwinLinkMathEx.h"

static const int ZFXY_1M_ZOOM_BASE = 25;
static const FIntVector ZFXY_ROOT_TILE(0);

namespace {

    constexpr double LIMIT_HEIGHT = 1.0 * (1 << FTwinLinkSpatialID::MAX_ZOOM_LEVEL);
    struct LngLat {
        double Lng;
        double Lat;
    };

    struct LngLatAlt {
        double Lng;
        double Lat;
        double Alt;
    };

    double GetFloor(int InF, int InZ) {
        return InF * FMath::Pow(2.0, ZFXY_1M_ZOOM_BASE) / FMath::Pow(2.0, InZ);
    }

    LngLat CalcLngLat(int InX, int InY, int InZ) {
        // LatとLng逆？　計算結果を見て判断

        const auto N = PI - 2.0 * PI * InY / FMath::Pow(2.0, InZ);
        const auto Lng = InX / FMath::Pow(2.0, InZ) * 360 - 180;
        const auto Lat = FMath::RadiansToDegrees(
            FMath::Atan(0.5 * (FMath::Exp(N) - FMath::Exp(-N))));
        return LngLat{ Lng, Lat };
    }

    double CalcAlt(int InZ, int InF) {
        return GetFloor(InF, InZ) +
            (FMath::Pow(2.0, ZFXY_1M_ZOOM_BASE) / FMath::Pow(2.0, InZ + 1));
    }

    LngLat CalcCenterLngLat(int InX, int InY, int InZ) {
        const auto _X = InX * 2 + 1;
        const auto _Y = InY * 2 + 1;
        const auto _Z = InZ + 1;
        return CalcLngLat(_X, _Y, _Z);
    }

    LngLatAlt CalcCenterLngLatAlt(int InX, int InY, int InZ, int InF) {
        const auto LngLat = CalcCenterLngLat(InX, InY, InZ);
        const auto Alt = CalcAlt(InZ, InF);
        return LngLatAlt{ LngLat.Lng, LngLat.Lat, Alt };
    }

}

FTwinLinkSpatialID::FTwinLinkSpatialID()
    : bIsValidAltitude(false)
    , Z(0.0)
    , F(0.0)
    , X(0.0)
    , Y(0.0) {
}

FTwinLinkSpatialID::~FTwinLinkSpatialID() {
}

bool FTwinLinkSpatialID::IsValid(int InZ, int InF, int InX, int InY, bool bIsUsingAltitude) {
    // ...
    return true;
}

FTwinLinkSpatialID FTwinLinkSpatialID::Create(double Lat, double Lng, double Zoom, double Alt, bool bIsUsingAltitude) {

    if (bIsUsingAltitude == false)
        Alt = 0.0;

    const auto Meters = Alt;
    if (Meters <= -FMath::Pow(2.0, (double)ZFXY_1M_ZOOM_BASE) ||
        Meters >= FMath::Pow(2.0, (double)ZFXY_1M_ZOOM_BASE)) {
        // todo : make altitude unlimited?
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Not supports altitude"));
        check(false);
    }

    // 空間IDに変換
    int _F = 0;
    if (bIsUsingAltitude)
        _F = FMath::Floor(((FMath::Pow(2.0, Zoom) * Meters) / FMath::Pow(2.0, (double)ZFXY_1M_ZOOM_BASE)));

    const auto D2r = PI / 180;
    const auto SinNumber = FMath::Sin(Lat * D2r);
    const auto Z2 = FMath::Pow(2.0, Zoom);
    double _X = Z2 * (Lng / 360.0 + 0.5);
    const auto _Y = Z2 * (0.5 - 0.25 * FMath::Loge((1 + SinNumber) / (1 - SinNumber)) / PI);

    _X = FMath::Fmod(_X, Z2);
    if (_X < 0)
        _X = _X + Z2;

    // 値の設定
    FTwinLinkSpatialID SpatialID;
    SpatialID.F = _F;
    SpatialID.X = FMath::Floor(_X);
    SpatialID.Y = FMath::Floor(_Y);
    SpatialID.Z = (int)Zoom;
    SpatialID.bIsValidAltitude = bIsUsingAltitude;
    return SpatialID;
}

FVector FTwinLinkSpatialID::ToVoxelSpace(const FPLATEAUGeoCoordinate& GeoCoordinate, int Zoom) {
    checkf(-LIMIT_HEIGHT <= GeoCoordinate.Height && GeoCoordinate.Height <= LIMIT_HEIGHT, TEXT("GeoCoodinate. Height is limit over"));

    // 空間IDに変換
    const auto _F = FMath::Pow(2.0, Zoom - ZFXY_1M_ZOOM_BASE) * GeoCoordinate.Height;

    const auto D2r = PI / 180;
    const auto SinNumber = FMath::Sin(GeoCoordinate.Latitude * D2r);
    const auto Z2 = FMath::Pow(2.0, Zoom);
    double _X = Z2 * (GeoCoordinate.Longitude / 360.0 + 0.5);
    const auto _Y = Z2 * (0.5 - 0.25 * FMath::Loge((1 + SinNumber) / (1 - SinNumber)) / PI);

    _X = FMath::Fmod(_X, Z2);
    if (_X < 0)
        _X = _X + Z2;
    FVector XYF;
    XYF.X = _X;
    XYF.Y = _Y;
    XYF.Z = _F;
    return XYF;
}

FVector FTwinLinkSpatialID::WorldToVoxelSpace(const FVector& World, FPLATEAUGeoReference& GeoReference, int Zoom) {
    const auto Geo = UPLATEAUGeoReferenceBlueprintLibrary::Unproject(GeoReference, World);
    return ToVoxelSpace(Geo, Zoom);
}

FTwinLinkSpatialID FTwinLinkSpatialID::Create(int InZ, int InF, int InX, int InY, bool bIsUsingAltitude) {
    check(IsValid(InZ, InF, InX, InY, bIsUsingAltitude));

    if (bIsUsingAltitude == false)
        InF = 0;

    FTwinLinkSpatialID SpatialID;
    SpatialID.Z = InZ;
    SpatialID.F = InF;
    SpatialID.X = InX;
    SpatialID.Y = InY;
    SpatialID.bIsValidAltitude = bIsUsingAltitude;
    return SpatialID;
}

FTwinLinkSpatialID FTwinLinkSpatialID::Create(FPLATEAUGeoReference& GeoReference, const FVector& Position, int ZoomLevel, bool bIsUsingAltitude) {
    const auto GeoCoordinate = UPLATEAUGeoReferenceBlueprintLibrary::Unproject(GeoReference, Position);
    return Create(GeoCoordinate.Latitude, GeoCoordinate.Longitude, (double)ZoomLevel, GeoCoordinate.Height, bIsUsingAltitude);
}

bool FTwinLinkSpatialID::TryGetBoudingSpatialId(FPLATEAUGeoReference& GeoReference, const FBox& WorldBox,
    bool bIsUsingAltitude, FTwinLinkSpatialID& Out)
{
    const auto Min = WorldToVoxelSpace(WorldBox.Min, GeoReference, MAX_ZOOM_LEVEL);
    const auto Max = WorldToVoxelSpace(WorldBox.Max, GeoReference, MAX_ZOOM_LEVEL);
    const auto [MinX, MinY, MinZ] = TwinLinkMathEx::FloorToInt64(Min.X, Min.Y, Min.Z);
    const auto [MaxX, MaxY, MaxZ] = TwinLinkMathEx::FloorToInt64(Max.X, Max.Y, Max.Z);
 
    const auto X = 64 - TwinLinkMathEx::Nlz(MinX ^ MaxX);
    const auto Y = 64 - TwinLinkMathEx::Nlz(MinY ^ MaxY);
    const auto Z = 64 - TwinLinkMathEx::Nlz(bIsUsingAltitude ? (MinZ ^ MaxZ) : 0u);

    const auto Zoom = MAX_ZOOM_LEVEL - FMath::Max3(X, Y, Z);
    if (Zoom < 0)
        return false;
    Out = Create(MAX_ZOOM_LEVEL, MinZ, MinX, MinY,  bIsUsingAltitude).ZoomChanged(Zoom);
    return true;
}

FTwinLinkSpatialID FTwinLinkSpatialID::ParseZFXY(const FString& Str) {
    // 数が合わない場合は失敗
    TArray<FString> Out;
    Str.ParseIntoArray(Out, TEXT("/"));
    if (Out.Num() != 4)
        return FTwinLinkSpatialID();

    // 数字じゃないものがあればパース失敗
    if (std::any_of(Out.begin(), Out.end(), [](const FString& s) { return s.IsNumeric() == false; }))
        return FTwinLinkSpatialID();
    const auto Z = FCString::Atoi(*Out[0]);
    const auto F = FCString::Atoi(*Out[1]);
    const auto X = FCString::Atoi(*Out[2]);
    const auto Y = FCString::Atoi(*Out[3]);
    return Create(Z, F, X, Y, true);
}

FBox FTwinLinkSpatialID::GetSpatialIDArea(FPLATEAUGeoReference& GeoReference) const {
    // north west
    const auto NW = CalcLngLat(X, Y, Z);
    const auto NWAlt = bIsValidAltitude ? CalcAlt(Z, F) : 0.0;
    // south east
    const auto SE = CalcLngLat(X + 1, Y + 1, Z);
    const auto SEAlt = bIsValidAltitude ? CalcAlt(Z, F + 1) : 0.0;

    FPLATEAUGeoCoordinate GeoCoordinate1;
    GeoCoordinate1.Latitude = NW.Lat;
    GeoCoordinate1.Longitude = NW.Lng;
    GeoCoordinate1.Height = NWAlt;

    FPLATEAUGeoCoordinate GeoCoordinate2;
    GeoCoordinate2.Latitude = SE.Lat;
    GeoCoordinate2.Longitude = SE.Lng;
    GeoCoordinate2.Height = SEAlt;

    const auto Point1 = UPLATEAUGeoReferenceBlueprintLibrary::Project(GeoReference, GeoCoordinate1);
    const auto Point2 = UPLATEAUGeoReferenceBlueprintLibrary::Project(GeoReference, GeoCoordinate2);

    return FBox(Point1, Point2);
}

FString FTwinLinkSpatialID::StringZFXY() const {
    return FString::Printf(TEXT("%d/%d/%d/%d"), Z, F, X, Y);
}

FTwinLinkSpatialID FTwinLinkSpatialID::ZoomChanged(int Zoom) const {
    FPLATEAUGeoCoordinate Geo;
    ToGeoCoordinate(Geo);
    return Create(Geo.Latitude, Geo.Longitude, (double)Zoom, Geo.Height, bIsValidAltitude);
}

void FTwinLinkSpatialID::ToGeoCoordinate(FPLATEAUGeoCoordinate& Out) const
{
    const auto NW = CalcLngLat(X, Y, Z);
    const auto NWAlt = bIsValidAltitude ? CalcAlt(Z, F) : 0.0;
    Out.Latitude = NW.Lat;
    Out.Longitude = NW.Lng;
    Out.Height = NWAlt;
}


FTwinLinkSpatialID FTwinLinkSpatialID::NorthWest() const {
    return *this;
}

FTwinLinkSpatialID FTwinLinkSpatialID::SouthEast() const {
    return Create(Z, F, X + 1, Y + 1, bIsValidAltitude);
}

FVector FTwinLinkSpatialID::ToVector() const {
    return FVector(1.0 * X, 1.0 * Y, 1.0 * F);
}
