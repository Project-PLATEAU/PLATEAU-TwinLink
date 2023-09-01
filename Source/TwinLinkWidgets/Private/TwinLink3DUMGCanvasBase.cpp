// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLink3DUMGCanvasBase.h"

#include "TwinLinkCommon.h"
#include "TwinLinkFacilityInfoSystem.h"
//#include "TwinLinkFacilityInfo.h"

void UTwinLink3DUMGCanvasBase::Setup(UTwinLinkObservableCollection* Collection) {
    ObservableCollection = Collection;
    check(ObservableCollection.IsValid());

    // 現在のコレクションの状態を同期
    OnPostReset();
    const auto CollectionRaw = ObservableCollection.Get()->GetCollectionRaw();
    for (const auto Raw : CollectionRaw) {
        OnPostAddElement(Raw);
    }

    // 要素が追加された時にUI側の処理を行う
    ObservableCollection.Get()->EvOnPostAdd.AddLambda(
        [this](const TWeakObjectPtr<UObject> Obj) {
            if (Obj.IsValid()) {
                OnPostAddElement(Obj.Get());
            }
        });

    // 要素が削除された時にUI側の処理を行う
    ObservableCollection.Get()->EvOnPostRemove.AddLambda(
        [this](const TWeakObjectPtr<UObject> Obj) {
            if (Obj.IsValid()) {
                OnPostRemoveElement(Obj.Get());
            }
        });

    // 要素がリセットされた時にUI側の処理を行う
    ObservableCollection.Get()->EvOnPostReset.AddLambda(
        [this]() {
            OnPostReset();
        });

}
