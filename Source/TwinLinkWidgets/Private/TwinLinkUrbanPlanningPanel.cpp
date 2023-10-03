// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkUrbanPlanningPanel.h"
#include "TwinLinkCommon.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TwinLinkUrbanPlanningViewSystem.h"
#include "TwinLinkUrbanPlanningElement.h"

void UTwinLinkUrbanPlanningPanel::TwinLinkUrbanPlanningUpdateGuide() {
    auto UrbanPlanningViewSys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkUrbanPlanningViewSystem>();
    check(UrbanPlanningViewSys.IsValid());
    const auto Guides = UrbanPlanningViewSys.Get()->GetGuideUsageColors();
    GuidesParent->ClearChildren();
    const FString Path = "/PLATEAU-TwinLink/Widgets/FacilityAttribute/WBP_TwinLinkUrbanPlanningElement.WBP_TwinLinkUrbanPlanningElement_C";
    TSubclassOf<class UUserWidget> WidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(*Path)).LoadSynchronous();
    for (const auto& Guide : Guides) {
        auto Element = CreateWidget<UTwinLinkUrbanPlanningElement>(GetWorld(), WidgetClass);
        Element->GuidesColor->SetColorAndOpacity(Guide.Value);
        Element->GuidesText->SetText(FText::FromString(Guide.Key));
        GuidesParent->AddChild(Cast<UWidget>(Element));
    }
}