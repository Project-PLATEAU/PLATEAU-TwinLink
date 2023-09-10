// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityInfoCollection.h"
#include "TwinLinkFacilityInfo.h"

void UTwinLinkFacilityInfoCollection::OnAdd(UObject* Obj) {
    auto FacilityInfo = Cast<UTwinLinkFacilityInfo>(Obj);
    check(FacilityInfo);
    FacilityInfoCollection.Add(FacilityInfo->GetUniqueID(), FacilityInfo);

}

void UTwinLinkFacilityInfoCollection::OnRemove(const TWeakObjectPtr<UObject>& Obj) {
    check(Obj.IsValid());
    auto FacilityInfo = Cast<UTwinLinkFacilityInfo>(Obj.Get());
    check(FacilityInfo);
    FacilityInfoCollection.Remove(FacilityInfo->GetUniqueID());

}

void UTwinLinkFacilityInfoCollection::OnReset() {
    FacilityInfoCollection.Reset();

}

bool UTwinLinkFacilityInfoCollection::OnContains(const TWeakObjectPtr<UObject>& Obj) {
    check(Obj.IsValid());
    const auto FacilityInfo = Cast<UTwinLinkFacilityInfo>(Obj.Get());
    check(FacilityInfo);
    return FacilityInfoCollection.Contains(FacilityInfo->GetUniqueID());
}

TArray<UObject*> UTwinLinkFacilityInfoCollection::OnGetCollectionRaw() {
    TArray<UObject*> RetCollection;
    RetCollection.Reserve(FacilityInfoCollection.Num());
    for (auto& FacilityInfo : FacilityInfoCollection) {
        RetCollection.Add(FacilityInfo.Value);
    }
    return RetCollection;
}
