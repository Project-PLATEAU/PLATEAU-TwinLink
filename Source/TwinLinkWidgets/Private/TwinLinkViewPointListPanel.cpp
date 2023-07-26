// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkViewPointListPanel.h"
#include "TwinLinkCommon.h"
#include "TwinLinkViewPointSystem.h"

void UTwinLinkViewPointListPanel::OnSetupOnTwinLink(TWeakObjectPtr<UTwinLinkObservableCollection>* OutCollection) const {
    auto ViewPointSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkViewPointSystem>();
    check(ViewPointSys.IsValid());
    *OutCollection = ViewPointSys.Get()->GetViewPointCollection().Get();
}
