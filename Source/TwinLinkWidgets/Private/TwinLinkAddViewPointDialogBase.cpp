// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkAddViewPointDialogBase.h"
#include "TwinLinkCommon.h"
#include "TwinLinkViewPointSystem.h"

#include "TwinLinkTickSystem.h"

void UTwinLinkAddViewPointDialogBase::AddViewPointInfo(const FString& ViewPointName) {
    const auto ViewPointSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkViewPointSystem>();

    if (ViewPointSystem->CheckAddableViewPointInfo(ViewPointName) == false) {
        OnFailedAddViewPointInfo(ViewPointName);
        return;
    }
    
    ViewPointSystem->AddViewPointInfo(ViewPointName);
    ViewPointSystem->ExportViewPointInfo();

    OnAddedViewPointInfo(ViewPointName);
}
