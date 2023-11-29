#include "TwinLinkPLATEAUGeoReferenceEx.h"

#include "TwinLinkPLATEAUCityModelEx.h"
#include "TwinLinkPLATEAUCityObjectGroupEx.h"

inline FBox FTwinLinkPLATEAUGeoReferenceEx::GetSpatialBox(FPLATEAUGeoReference& Self,
    const UPLATEAUCityObjectGroup* CityModel, int Zoom) {
    FBox Bb;
    if (FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(CityModel, Bb) == false)
        return FBox(FVector::Zero(), FVector::Zero());

    const auto Min = FTwinLinkSpatialID::WorldToVoxelSpace(Bb.Min, Self, Zoom);
    const auto Max = FTwinLinkSpatialID::WorldToVoxelSpace(Bb.Max, Self, Zoom);
    return FBox(Min, Max);
}
