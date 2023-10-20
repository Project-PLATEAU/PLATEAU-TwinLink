#pragma once
#include "PLATEAUInstancedCityModel.h"

class TWINLINK_API FTwinLinkPLATEAUCityObjectGroupEx
{
public:
    /*
     * @brief : BoundingBox取得
     */
    static bool TryBoundingBox(const UPLATEAUCityObjectGroup* Self, FBox& Out);
};
