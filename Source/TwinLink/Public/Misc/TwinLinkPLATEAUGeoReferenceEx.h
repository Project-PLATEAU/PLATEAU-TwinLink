#pragma once
#include "PLATEAUInstancedCityModel.h"
#include "TwinLinkSpatialID.h"
class APlayerController;
class USceneComponent;

class TWINLINK_API FTwinLinkPLATEAUGeoReferenceEx {
public:
    
    /*
     * @brief Selfをもとに、CityModelのバウンディングボックスを空間ID座標系に変換する
     */
    static FBox GetSpatialBox(FPLATEAUGeoReference& Self, const UPLATEAUCityObjectGroup* CityModel, int Zoom);
};
