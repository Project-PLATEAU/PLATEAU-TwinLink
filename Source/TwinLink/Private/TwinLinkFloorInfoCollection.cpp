// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFloorInfoCollection.h"
#include "TwinLinkFloorInfo.h"

void UTwinLinkFloorInfoCollection::OnAdd(UObject* Obj) {
    auto FloorInfo = Cast<UTwinLinkFloorInfo>(Obj);
    check(FloorInfo);
    FloorInfoCollection.Add(FloorInfo->GetUniqueID(), FloorInfo);

}

void UTwinLinkFloorInfoCollection::OnRemove(const TWeakObjectPtr<UObject>& Obj) {
    check(Obj.IsValid());
    auto FloorInfo = Cast<UTwinLinkFloorInfo>(Obj.Get());
    check(FloorInfo);
    FloorInfoCollection.Remove(FloorInfo->GetUniqueID());

}

void UTwinLinkFloorInfoCollection::OnReset() {
    FloorInfoCollection.Reset();

}

bool UTwinLinkFloorInfoCollection::OnContains(const TWeakObjectPtr<UObject>& Obj) {
    check(Obj.IsValid());
    const auto FloorInfo = Cast<UTwinLinkFloorInfo>(Obj.Get());
    check(FloorInfo);
    return FloorInfoCollection.Contains(FloorInfo->GetUniqueID());

}

TArray<UObject*> UTwinLinkFloorInfoCollection::OnGetCollectionRaw() {
    TArray<UObject*> RetCollection;
    RetCollection.Reserve(FloorInfoCollection.Num());
    for (auto& FloorInfo : FloorInfoCollection) {
        RetCollection.Add(FloorInfo.Value);
    }
    return RetCollection;

}

void UTwinLinkFloorInfoCollection::SetFloorVisible(bool V) {
    this->bFloorVisible = V;
}

bool UTwinLinkFloorInfoCollection::IsFloorVisible() const {
    return bFloorVisible;
}
