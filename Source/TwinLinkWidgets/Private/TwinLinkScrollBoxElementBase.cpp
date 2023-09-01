// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkScrollBoxElementBase.h"
#include "TwinLinkCommon.h"
#include "TwinLinkTickSystem.h"

#include "TwinLinkScrollBoxElementImpl.h"

void UTwinLinkScrollBoxElementBase::Setup(UTwinLinkScrollBoxElementImpl* _Impl, UObject* Obj) {
    check(_Impl);
    check(Obj);
    WidgetData = Obj;

    Impl = _Impl;
    check(Impl != nullptr);

    if (Impl.IsValid())
        Impl->OnSetup(WidgetData.Get());

    //// 登録済みのイベントを削除
    //if (EvOnChangedHnd.IsValid()) {
    //    CastedElement->EvOnChanged.Remove(EvOnChangedHnd);
    //    EvOnChangedHnd.Reset();
    //}

    //EvOnChangedHnd = CastedElement->EvOnChanged.AddLambda([]() {
    //    Onchanged
    //    });

    OnChangedElement();
}

void UTwinLinkScrollBoxElementBase::RequestRemove() const {
    check(WidgetData.IsValid());

    const auto TickSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkTickSystem>();
    check(TickSystem.IsValid());

    // 遅延削除　BP側でこの関数を呼び出した際にBP側の処理が完了するまで正常な値を保証するため
    TickSystem.Get()->EvDelayExec.AddLambda([this]() {
        if (Impl.IsValid())
            Impl->OnRequestRemove(WidgetData.Get());
        });
}

void UTwinLinkScrollBoxElementBase::RequestSelect() const {
    check(WidgetData.IsValid());
    if (Impl.IsValid())
        Impl->OnRequestSelect(WidgetData.Get());

}

void UTwinLinkScrollBoxElementBase::RequestEditName(const FString& NewName) const {
    check(WidgetData.IsValid());
    if (Impl.IsValid())
        Impl->OnRequestEdit(WidgetData.Get(), NewName);
}

const UObject* UTwinLinkScrollBoxElementBase::GetElement() const {
    check(WidgetData.IsValid());
    return WidgetData.Get();
}

const FText UTwinLinkScrollBoxElementBase::GetName() const {
    check(WidgetData.IsValid());

    if (Impl.IsValid())
        return Impl->OnGetName(WidgetData.Get());

    check(false);
    return FText::FromString(TEXT("undefind"));
}
