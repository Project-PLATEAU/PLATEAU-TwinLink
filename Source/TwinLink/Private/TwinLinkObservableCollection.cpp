// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkObservableCollection.h"

void UTwinLinkObservableCollection::Add(UObject* Obj) {
    OnAdd(Obj);

    if (EvOnPostAdd.IsBound()) {
        EvOnPostAdd.Broadcast(Obj);
    }
}

void UTwinLinkObservableCollection::Remove(const TWeakObjectPtr<UObject> Obj) {
    OnRemove(Obj);

    if (EvOnPostRemove.IsBound()) {
        EvOnPostRemove.Broadcast(Obj);
    }
}

void UTwinLinkObservableCollection::Reset() {
    OnReset();

    if (EvOnPostReset.IsBound()) {
        EvOnPostReset.Broadcast();
    }
}

bool UTwinLinkObservableCollection::Contains(const TWeakObjectPtr<UObject> Obj) {
    return OnContains(Obj);
}

TArray<UObject*> UTwinLinkObservableCollection::GetCollectionRaw() {
    return OnGetCollectionRaw();
}
