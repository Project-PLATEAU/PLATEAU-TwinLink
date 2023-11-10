// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkPOIElement.h"
#include "TwinLinkCommon.h"
#include "TwinLinkPOISubSystem.h"

void UTwinLinkPOIElement::TwinLinkPOIRemove() {
    auto POISubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
    check(POISubSystem.IsValid());

    const auto Name = AttributeText->GetText().ToString();

    const auto Actors = POISubSystem.Get()->POIActors[Name];

    for (const auto& Actor : Actors) {
        Actor->Destroy();
    }

    POISubSystem.Get()->POIActors.Remove(Name);

    POISubSystem.Get()->RegisteredPOIs.Remove(Name);

    POISubSystem.Get()->MaterialCollection.Remove(Name);

    POISubSystem.Get()->AttributeNames.Remove(Name);

    POISubSystem.Get()->AssignColors.Remove(Color);

    RemoveFromParent();
}

void UTwinLinkPOIElement::TwinLinkPOIVisible() {
    auto POISubSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkPOISubSystem>();
    check(POISubSystem.IsValid());

    const auto Name = AttributeText->GetText().ToString();

    const auto Actors = POISubSystem.Get()->POIActors[Name];

    for (const auto& Actor : Actors) {
        Actor->SetActorHiddenInGame(!Actor->IsHidden());
    }

}
