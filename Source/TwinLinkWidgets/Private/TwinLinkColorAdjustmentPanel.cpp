// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkColorAdjustmentPanel.h"
#include "TwinLinkCommon.h"
#include "TwinLinkColorAdjustmentElement.h"
#include "TwinLinkColorAdjustmentSystem.h"
#include "TwinLinkMapEditingBase.h"

void UTwinLinkColorAdjustmentPanel::TwinLinkColorAdjustmentInit() {
    auto ColorAdjustmentSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkColorAdjustmentSystem>();
    check(ColorAdjustmentSystem.IsValid());
    if (!ColorAdjustmentSystem.Get()->Materials.IsEmpty()) {
        return;
    }
    ColorAdjustmentSystem.Get()->Materials.Reset();
    ColorAdjustmentSystem.Get()->ExtractMaterial();
    Summary->ClearChildren();
    const FString Path = "/PLATEAU-TwinLink/Widgets/MapEdit/WBP_TwinLinkColorAdjustmentElement.WBP_TwinLinkColorAdjustmentElement_C";
    TSubclassOf<class UUserWidget> WidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(*Path)).LoadSynchronous();
    for (const auto& Material : ColorAdjustmentSystem.Get()->Materials) {
        int Index = &Material - &ColorAdjustmentSystem.Get()->Materials[0];
        auto Element = CreateWidget<UTwinLinkColorAdjustmentElement>(GetWorld(), WidgetClass);
        Element->MaterialPreview->SetColorAndOpacity(ColorAdjustmentSystem.Get()->MaterialParameter[Index].BaseColor);
        Element->MaterialName->SetText(FText::FromString(TEXT("マテリアル" + FString::FromInt(Index + 1))));
        Element->MaterialEdit->OnClicked.AddDynamic(Element, &UTwinLinkColorAdjustmentElement::OnSelectEdit);
        Element->ElementRoot = this;
        Element->MaterialIndex = Index;
        Summary->AddChild(Cast<UWidget>(Element));
    }
    //階層選択との繋ぎこみ
    const auto Floor = Cast<UTwinLinkMapEditingBase>(GetOuter()->GetOuter())->Floor;
    Floor->OnChangeFloorDelegate.Clear();
    Floor->OnChangeFloorDelegate.AddDynamic(this, &UTwinLinkColorAdjustmentPanel::OnSelectFloor);
}

void UTwinLinkColorAdjustmentPanel::EditMaterial(int MaterialIndex) {
    auto ColorAdjustmentSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkColorAdjustmentSystem>();
    check(ColorAdjustmentSystem.IsValid());

    if (Editor->GetVisibility() == ESlateVisibility::Visible) {
        Editor->OnClickedClose();
    }

    Editor->SetupEditor(MaterialIndex);
    Editor->SetVisibility(ESlateVisibility::Visible);

    Editor->Thumbnail = Cast<UTwinLinkColorAdjustmentElement>(Summary->GetChildAt(MaterialIndex))->MaterialPreview;
}

void UTwinLinkColorAdjustmentPanel::OnSelectFloor(UPLATEAUCityObjectGroup* Value) {
    auto ColorAdjustmentSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkColorAdjustmentSystem>();
    check(ColorAdjustmentSystem.IsValid());

    if (Editor->GetVisibility() == ESlateVisibility::Visible) {
        Editor->OnClickedClose();
    }

    TArray<int> Enables;
    const auto Materials = Value->GetMaterials();
    for (const auto& Material : Materials) {
        const auto Index = ColorAdjustmentSystem.Get()->Materials.Find(Material);
        if (Index != -1) {
            Enables.Add(Index);
        }
    }
    SummaryFilter(Enables);
}

void UTwinLinkColorAdjustmentPanel::TwinLinkColorAdjustmentExit() {
    if (Editor->GetVisibility() == ESlateVisibility::Visible) {
        Editor->OnClickedClose();
    }
}

void UTwinLinkColorAdjustmentPanel::SummaryFilter(const TArray<int>& EnableList) {
    for (int i = 0; i < Summary->GetChildrenCount(); i++) {
        Summary->GetChildAt(i)->SetVisibility(EnableList.Find(i) != -1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}