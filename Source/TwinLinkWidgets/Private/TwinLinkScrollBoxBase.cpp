// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkScrollBoxBase.h"

#include "Components/ScrollBox.h"

#include "TwinLinkCommon.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkTickSystem.h"
#include "TwinLinkScrollBoxElementBase.h"

void UTwinLinkScrollBoxBase::Setup(
    UTwinLinkObservableCollection* Collection,
    UTwinLinkScrollBoxElementImpl* _ElmentImpl) {
    // 既に登録済みのコレクションがあるなら連携を切る
    if (ObservableCollection.IsValid()) {
        ObservableCollection->EvOnPostAdd.Remove(EvOnPostAddHnd);
        ObservableCollection->EvOnPostRemove.Remove(EvOnPostRemoveHnd);
        ObservableCollection->EvOnPostReset.Remove(EvOnPostResetHnd);
    }

    ObservableCollection = Collection;
    check(ObservableCollection.IsValid());

    check(_ElmentImpl);
    ElmentImpl = _ElmentImpl;

    // 現在のコレクションの状態を同期
    OnPostReset();
    const auto CollectionRaw = ObservableCollection.Get()->GetCollectionRaw();
    for (const auto Raw : CollectionRaw) {
        OnPostAddElement(Raw);
    }

    // 要素が追加された時にUI側の処理を行う
    EvOnPostAddHnd = ObservableCollection.Get()->EvOnPostAdd.AddLambda(
        [this](const TWeakObjectPtr<UObject> Obj) {
            if (Obj.IsValid()) {
                OnPostAddElement(Obj.Get());
            }
        });

    // 要素が削除された時にUI側の処理を行う
    EvOnPostRemoveHnd = ObservableCollection.Get()->EvOnPostRemove.AddLambda(
        [this](const TWeakObjectPtr<UObject> Obj) {
            if (Obj.IsValid()) {
                OnPostRemoveElement(Obj.Get());
            }
        });

    // 要素がリセットされた時にUI側の処理を行う
    EvOnPostResetHnd = ObservableCollection.Get()->EvOnPostReset.AddLambda(
        [this]() {
            OnPostReset();
        });

}

TArray<UObject*> UTwinLinkScrollBoxBase::GetCollection() const {
    return ObservableCollection->GetCollectionRaw();
}

UTwinLinkScrollBoxElementBase* UTwinLinkScrollBoxBase::FindChild(UObject* Elements) {
    check(Elements);
    const auto RetPtr = ScrollBoxElementWidgets.Find(Elements->GetUniqueID());
    check(RetPtr);
    return *RetPtr;
}

void UTwinLinkScrollBoxBase::AddElementWidgets(UTwinLinkScrollBoxElementBase* Widget) {
    ScrollBoxElementWidgets.Add(Widget->GetElement()->GetUniqueID(), Widget);
}

void UTwinLinkScrollBoxBase::RemoveElementWidgets(UTwinLinkScrollBoxElementBase* Widget) {
    ScrollBoxElementWidgets.Remove(Widget->GetElement()->GetUniqueID());
}

void UTwinLinkScrollBoxBase::ResetElementWidgets() {
    ScrollBoxElementWidgets.Reset();
}
