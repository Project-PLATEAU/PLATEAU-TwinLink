// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkViewPointInfoCollection.h"
#include "TwinLinkViewPointInfo.h"

void UTwinLinkViewPointInfoCollection::OnAdd(UObject* Obj) {
    auto ViewPointInfo = Cast<UTwinLinkViewPointInfo>(Obj);
    check(ViewPointInfo);
    ViewPointInfoCollection.Add(ViewPointInfo->GetUniqueID(), ViewPointInfo);
}

void UTwinLinkViewPointInfoCollection::OnRemove(const TWeakObjectPtr<UObject> Obj) {
    check(Obj.IsValid());
    auto ViewPointInfo = Cast<UTwinLinkViewPointInfo>(Obj.Get());
    check(ViewPointInfo);
    ViewPointInfoCollection.Remove(ViewPointInfo->GetUniqueID());

}

void UTwinLinkViewPointInfoCollection::OnReset() {
    ViewPointInfoCollection.Reset();
}

bool UTwinLinkViewPointInfoCollection::OnContains(const TWeakObjectPtr<UObject> Obj) {
    check(Obj.IsValid());
    const auto ViewPointInfo = Cast<UTwinLinkViewPointInfo>(Obj.Get());
    check(ViewPointInfo);
    return ViewPointInfoCollection.Contains(ViewPointInfo->GetUniqueID());
}

TArray<UObject*> UTwinLinkViewPointInfoCollection::OnGetCollectionRaw() {
    TArray<UObject*> RetCollection;
    RetCollection.Reserve(ViewPointInfoCollection.Num());
    for (auto& ViewPointInfo : ViewPointInfoCollection) {
        RetCollection.Add(ViewPointInfo.Value);
    }
    return RetCollection;
}
