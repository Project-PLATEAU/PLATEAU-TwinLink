// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkRoom3DNameInstBase.h"
#include "TwinLinkCommon.h"

#include "TwinLinkObservableCollection.h"

void UTwinLinkRoom3DNameInstBase::Setup(UTwinLinkObservableCollection* Collection) {
    check(Collection != nullptr);

    if (ObservableCollection.IsValid()) {
        Reset();
    }

    ObservableCollection = Collection;

    // 要素が追加された時にUI側の処理を行う
    OnPostAddElementHnd = ObservableCollection.Get()->EvOnPostAdd.AddLambda(
        [this](const TWeakObjectPtr<UObject> Obj) {
            if (Obj.IsValid()) {
                OnPostAddElement(Obj.Get());
            }
        });

    // 要素が削除された時にUI側の処理を行う
    OnPostRemoveElementHnd = ObservableCollection.Get()->EvOnPostRemove.AddLambda(
        [this](const TWeakObjectPtr<UObject> Obj) {
            if (Obj.IsValid()) {
                OnPostRemoveElement(Obj.Get());
            }
        });

    // 要素がリセットされた時にUI側の処理を行う
    OnPostResetHnd = ObservableCollection.Get()->EvOnPostReset.AddLambda(
        [this]() {
            OnPostReset();
        });
}

void UTwinLinkRoom3DNameInstBase::Reset() {
    if (ObservableCollection.IsValid()) {
        OnPostReset();

        ObservableCollection.Get()->EvOnPostAdd.Remove(OnPostAddElementHnd);
        ObservableCollection.Get()->EvOnPostRemove.Remove(OnPostRemoveElementHnd);
        ObservableCollection.Get()->EvOnPostReset.Remove(OnPostResetHnd);

        ObservableCollection = nullptr;
    }
}

TArray<UObject*> UTwinLinkRoom3DNameInstBase::GetCollectionRaw() {
    return ObservableCollection->GetCollectionRaw();
}
