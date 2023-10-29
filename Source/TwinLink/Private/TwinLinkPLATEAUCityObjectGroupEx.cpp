#include "TwinLinkPLATEAUCityObjectGroupEx.h"

bool FTwinLinkPLATEAUCityObjectGroupEx::TryBoundingBox(const UPLATEAUCityObjectGroup* Self, FBox& Out) {
    auto Ret = GetBoundingBox(Self);
    if (Ret.has_value() == false)
        return false;
    Out = *Ret;
    return true;
}

std::optional<FBox> FTwinLinkPLATEAUCityObjectGroupEx::GetBoundingBox(const UPLATEAUCityObjectGroup* Self) {
    if (!Self)
        return std::nullopt;

    const auto Center = Self->Bounds.Origin;
    const auto Extent = Self->Bounds.BoxExtent;
    return FBox(Center - Extent, Center + Extent);
}

bool FTwinLinkPLATEAUCityObjectGroupEx::TryGetStaticMeshBoundingBox(const UPLATEAUCityObjectGroup* Self, FBox& Out) {
    auto Ret = GetStaticMeshBoundingBox(Self);
    if (Ret.has_value() == false)
        return false;
    Out = *Ret;
    return true;
}

std::optional<FBox> FTwinLinkPLATEAUCityObjectGroupEx::GetStaticMeshBoundingBox(const UPLATEAUCityObjectGroup* Self) {
    if (!Self)
        return std::nullopt;

    const auto StaMesh = Self->GetStaticMesh();
    if (!StaMesh)
        return std::nullopt;
    return StaMesh->GetBoundingBox();
}
