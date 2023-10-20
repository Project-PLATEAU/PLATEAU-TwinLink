#include "TwinLinkPLATEAUCityObjectGroupEx.h"

bool FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(const UPLATEAUCityObjectGroup* Self, FBox& Out) {
    if (!Self)
        return false;

    const auto StaMesh = Self->GetStaticMesh();
    if (!StaMesh)
        return false;
    Out = StaMesh->GetBoundingBox();
    return true;
}
