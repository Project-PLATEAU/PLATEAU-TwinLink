// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAssetPlacementInfoCollection.h"

void UTwinLinkAssetPlacementInfoCollection::OnAdd(UObject* Obj) {
    auto AssetPlacementInfo = Cast<UTwinLinkAssetPlacementInfo>(Obj);
    check(AssetPlacementInfo);
    AssetPlacementInfoCollection.Add(AssetPlacementInfo->GetUniqueID(), AssetPlacementInfo);
}

void UTwinLinkAssetPlacementInfoCollection::OnRemove(const TWeakObjectPtr<UObject>& Obj) {
    check(Obj.IsValid());
    auto AssetPlacementInfo = Cast<UTwinLinkAssetPlacementInfo>(Obj.Get());
    check(AssetPlacementInfo);
    AssetPlacementInfoCollection.Remove(AssetPlacementInfo->GetUniqueID());
}

void UTwinLinkAssetPlacementInfoCollection::OnReset() {
    AssetPlacementInfoCollection.Reset();
}

bool UTwinLinkAssetPlacementInfoCollection::OnContains(const TWeakObjectPtr<UObject>& Obj) {
    check(Obj.IsValid());
    const auto AssetPlacementInfo = Cast<UTwinLinkAssetPlacementInfo>(Obj.Get());
    check(AssetPlacementInfo);
    return AssetPlacementInfoCollection.Contains(AssetPlacementInfo->GetUniqueID());
}

TArray<UObject*> UTwinLinkAssetPlacementInfoCollection::OnGetCollectionRaw() {
    TArray<UObject*> RetCollection;
    RetCollection.Reserve(AssetPlacementInfoCollection.Num());
    for (auto& AssetPlacementInfo : AssetPlacementInfoCollection) {
        RetCollection.Add(AssetPlacementInfo.Value);
    }
    return RetCollection;
}

void UTwinLinkAssetPlacementInfoCollection::Update(const uint32 Key, const TWeakObjectPtr<UObject>& Obj) {
    auto AssetPlacementInfo = Cast<UTwinLinkAssetPlacementInfo>(Obj);
    check(AssetPlacementInfo);
    AssetPlacementInfoCollection.Add(Key, AssetPlacementInfo);
}

TObjectPtr<UTwinLinkAssetPlacementInfo>* UTwinLinkAssetPlacementInfoCollection::GetFromKey(uint32 Key) {
    return AssetPlacementInfoCollection.Find(Key);
}

uint32 UTwinLinkAssetPlacementInfoCollection::RemoveFromKey(uint32 Key) {
    return AssetPlacementInfoCollection.Remove(Key);
}