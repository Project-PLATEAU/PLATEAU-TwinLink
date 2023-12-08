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
    for (const auto& Function : UrbanPlanningViewSys.Get()->GetFunctionDefines()->UrbanPlanningFunctions) {
        if (!Guides.Contains(Function.FunctionName)) {
            continue;
        }
        auto Element = CreateWidget<UTwinLinkUrbanPlanningElement>(GetWorld(), WidgetClass);
        Element->GuidesColor->SetColorAndOpacity(Guides[Function.FunctionName]);
        Element->GuidesText->SetText(FText::FromString(Function.FunctionName));
        GuidesParent->AddChild(Cast<UWidget>(Element));
    }
}