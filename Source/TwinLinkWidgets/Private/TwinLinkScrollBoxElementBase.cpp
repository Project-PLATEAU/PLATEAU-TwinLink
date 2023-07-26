// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkScrollBoxElementBase.h"
#include "TwinLinkCommon.h"
#include "TwinLinkTickSystem.h"

void UTwinLinkScrollBoxElementBase::Setup(UObject* Obj) {
    check(Obj);
    WidgetData = Obj;
    Sync();
}

void UTwinLinkScrollBoxElementBase::RequestRemove() const {
    check(WidgetData.IsValid());

    const auto TickSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkTickSystem>();
    check(TickSystem.IsValid());
    
    // 遅延削除　BP側でこの関数を呼び出した際にBP側の処理が完了するまで正常な値を保証するため
    TickSystem.Get()->EvDelayExec.AddLambda([this]() {
            OnRequestRemove(WidgetData.Get());
        });
}

void UTwinLinkScrollBoxElementBase::RequestSelect() const {
    check(WidgetData.IsValid());
    OnRequestSelect(WidgetData.Get());
}

void UTwinLinkScrollBoxElementBase::RequestEditName(const FString NewName) const {
    OnRequestEdit(NewName);
}

const UObject* UTwinLinkScrollBoxElementBase::GetElement() const {
    check(WidgetData.IsValid());
    return WidgetData.Get();
}

const FText UTwinLinkScrollBoxElementBase::GetName() const {
    check(WidgetData.IsValid());
    return OnGetName(WidgetData.Get());
}
