// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFacilityInfo3DInstBase.h"

#include "TwinLinkCommon.h"
#include "TwinLinkFacilityInfoSystem.h"

// Sets default values for this component's properties
UTwinLinkFacilityInfo3DInstBase::UTwinLinkFacilityInfo3DInstBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UTwinLinkFacilityInfo3DInstBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
    auto FacilityInfoSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFacilityInfoSystem>();
    check(FacilityInfoSys.IsValid());
    auto Collection = FacilityInfoSys.Get()->GetFacilityInfoCollection().Get();
    check(Collection != nullptr);

    Setup(Collection);

}

void UTwinLinkFacilityInfo3DInstBase::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    Reset();
    Super::EndPlay(EndPlayReason);
}

void UTwinLinkFacilityInfo3DInstBase::Setup(UTwinLinkObservableCollection* Collection) {
    check(Collection != nullptr);

    if (ObservableCollection.IsValid()) {
        ObservableCollection.Get()->EvOnPostAdd.Remove(OnPostAddElementHnd);
        ObservableCollection.Get()->EvOnPostRemove.Remove(OnPostRemoveElementHnd);
        ObservableCollection.Get()->EvOnPostReset.Remove(OnPostResetHnd);
    }

    ObservableCollection = Collection;

    // 現在のコレクションの状態を同期
    OnPostReset();
    const auto CollectionRaw = ObservableCollection.Get()->GetCollectionRaw();
    for (const auto Raw : CollectionRaw) {
        OnPostAddElement(Raw);
    }

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

void UTwinLinkFacilityInfo3DInstBase::Reset() {
    if (ObservableCollection.IsValid()) {
        OnPostReset();

        ObservableCollection.Get()->EvOnPostAdd.Remove(OnPostAddElementHnd);
        ObservableCollection.Get()->EvOnPostRemove.Remove(OnPostRemoveElementHnd);
        ObservableCollection.Get()->EvOnPostReset.Remove(OnPostResetHnd);

        ObservableCollection = nullptr;
    }

}
