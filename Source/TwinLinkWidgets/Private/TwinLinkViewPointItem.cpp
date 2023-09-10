// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkViewPointItem.h"

#include "TwinLinkCommon.h"
#include "TwinLinkViewPointSystem.h"
#include "TwinLinkViewPointInfo.h"


void UTwinLinkViewPointItem::OnSetup(UObject* Data) const {
}

void UTwinLinkViewPointItem::OnRequestRemove(UObject* Data) const {
    const auto ViewPointSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkViewPointSystem>();
    check(ViewPointSystem.IsValid());

    auto CastedElement = Cast<UTwinLinkViewPointInfo>(Data);
    check(CastedElement);
    
    ViewPointSystem.Get()->RemoveViewPointInfo(CastedElement);
    ViewPointSystem.Get()->ExportViewPointInfo();

}

void UTwinLinkViewPointItem::OnRequestEdit(UObject* Data, const FString& NewName) const {
    const auto ViewPointSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkViewPointSystem>();
    check(ViewPointSystem.IsValid());

    auto CastedElement = Cast<UTwinLinkViewPointInfo>(Data);
    check(CastedElement);

    ViewPointSystem->RequestEditName(CastedElement, NewName);
    ViewPointSystem->ExportViewPointInfo();
}

FText UTwinLinkViewPointItem::OnGetName(UObject* Data) const {
    auto CastedData = Cast<UTwinLinkViewPointInfo>(Data);
    check(CastedData);
    return FText::FromString(CastedData->GetName());
}

void UTwinLinkViewPointItem::OnRequestSelect(UObject* Data) const
{
    const auto ViewPointSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkViewPointSystem>();
    check(ViewPointSystem.IsValid());

    auto CastedElement = Cast<UTwinLinkViewPointInfo>(Data);
    check(CastedElement);

    ViewPointSystem.Get()->ApplyViewPointInfo(CastedElement);

}
