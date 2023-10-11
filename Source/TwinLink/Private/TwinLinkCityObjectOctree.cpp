#pragma once
#include "TwinLinkCityObjectOctree.h"

FVector TwinLinkCityObjectTree::Node::GetPosition() const {
    // #TODO
    return CityObjectGroup->GetComponentLocation();
}

FVector TwinLinkCityObjectTree::Node::GetExtent() const {
    // #TODO
    return CityObjectGroup->GetNavigationBounds().GetExtent();
}

void TwinLinkCityObjectTree::Init(APLATEAUInstancedCityModel* CityModel) {
    auto& Geo = CityModel->GeoReference;
    auto& Data = Geo.GetData();
    InstancedCityModel = CityModel;
    FVector Origin;
    FVector Extent;
    // #TODO : グリッドサイズが完全な立方体のみっぽいので、Z方向の座標変換が必要になりそう.
    InstancedCityModel->GetActorBounds(false, Origin, Extent, true);
    Tree = new TreeType(Origin, Extent.X);
}

void TwinLinkCityObjectTree::Add(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject) {
    if (!Tree)
        return;
    if (CityObject.IsValid() == false)
        return;
    APLATEAUInstancedCityModel Model;
    Tree->AddElement(Node{ CityObject });
}
