// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkInfo3DPanelBase.h"
#include "TwinLinkCommon.h"

#include "TwinLinkObservableDataObjBase.h"
#include "TwinLinkName3DData.h"

void UTwinLinkInfo3DPanelBase::Setup(UObject* Info, FVector Offset) {
    InfoObservable = Cast<UTwinLinkObservableDataObjBase>(Info);
    check(InfoObservable.IsValid());
    InfoGetter = Cast<ITwinLinkName3DData>(Info);
    check(InfoGetter != nullptr);
    DisplayOffset = Offset;
}

FString UTwinLinkInfo3DPanelBase::GetInfo() const {
    return InfoGetter->GetName();
}

FVector UTwinLinkInfo3DPanelBase::GetPosition() const {
    return InfoGetter->GetPosition() + DisplayOffset;
}
