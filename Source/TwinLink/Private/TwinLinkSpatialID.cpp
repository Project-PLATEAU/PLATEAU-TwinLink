// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkSpatialID.h"
#include "TwinLinkCommon.h"

#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUGeometry.h"

static const int ZFXY_1M_ZOOM_BASE = 25;
static const FIntVector ZFXY_ROOT_TILE(0);

namespace {
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
