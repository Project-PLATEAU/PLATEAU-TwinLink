// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkColorAdjustmentEdit.h"
#include "TwinLinkCommon.h"
#include "TwinLinkColorAdjustmentSystem.h"
#include "Components/StaticMeshComponent.h"

void UTwinLinkColorAdjustmentEdit::SetupEditor(const int Index) {
    auto ColorAdjustmentSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkColorAdjustmentSystem>();
    check(ColorAdjustmentSystem.IsValid());

    MaterialIndex = Index;

    RefrectParameter(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex]);

    RevertMaterialParameter = ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex];

    SliderBaseColorR->OnValueChanged.Clear();
    SliderBaseColorG->OnValueChanged.Clear();
    SliderBaseColorB->OnValueChanged.Clear();
    SliderBaseColorR->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderBaseColorR);
    SliderBaseColorG->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderBaseColorG);
    SliderBaseColorB->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderBaseColorB);
    InputBaseColorR->OnTextChanged.Clear();
    InputBaseColorG->OnTextChanged.Clear();
    InputBaseColorB->OnTextChanged.Clear();
    InputBaseColorR->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputBaseColorR);
    InputBaseColorG->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputBaseColorG);
    InputBaseColorB->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputBaseColorB);

    SliderSpecularColorR->OnValueChanged.Clear();
    SliderSpecularColorR->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderSpecularColorR);
    InputSpecularColorR->OnTextChanged.Clear();
    InputSpecularColorR->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputSpecularColorR);

    SliderEmissiveColorR->OnValueChanged.Clear();
    SliderEmissiveColorG->OnValueChanged.Clear();
    SliderEmissiveColorB->OnValueChanged.Clear();
    SliderEmissiveColorR->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderEmissiveColorR);
    SliderEmissiveColorG->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderEmissiveColorG);
    SliderEmissiveColorB->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderEmissiveColorB);
    InputEmissiveColorR->OnTextChanged.Clear();
    InputEmissiveColorG->OnTextChanged.Clear();
    InputEmissiveColorB->OnTextChanged.Clear();
    InputEmissiveColorR->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputEmissiveColorR);
    InputEmissiveColorG->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputEmissiveColorG);
    InputEmissiveColorB->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputEmissiveColorB);

    SliderShininess->OnValueChanged.Clear();
    SliderShininess->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderShininess);
    InputShininess->OnTextChanged.Clear();
    InputShininess->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputShininess);

    SliderTransparency->OnValueChanged.Clear();
    SliderTransparency->OnValueChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnValueChangedSliderTransparency);
    InputTransparency->OnTextChanged.Clear();
    InputTransparency->OnTextChanged.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnTextChangedInputTransparency);

    CloseButton->OnClicked.Clear();
    CloseButton->OnClicked.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnClickedClose);

    ResetButton->OnClicked.Clear();
    ResetButton->OnClicked.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnClickedReset);

    RegistButton->OnClicked.Clear();
    RegistButton->OnClicked.AddDynamic(this, &UTwinLinkColorAdjustmentEdit::OnClickedRegist);

    SliderShininess->SetIsEnabled(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].MaterialType == ETwinLinkColorAdjustmentMaterialType::Opaque);
    InputShininess->SetIsReadOnly(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].MaterialType == ETwinLinkColorAdjustmentMaterialType::Transparent);

    SliderTransparency->SetIsEnabled(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].MaterialType == ETwinLinkColorAdjustmentMaterialType::Transparent);
    InputTransparency->SetIsReadOnly(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].MaterialType == ETwinLinkColorAdjustmentMaterialType::Opaque);
}

void UTwinLinkColorAdjustmentEdit::RefrectParameter(const FTwinLinkAdjustmentMaterialParameter& Param) {
    TextName->SetText(FText::FromString(Param.Name));

    SliderBaseColorR->SetValue(Param.BaseColor.R);
    SliderBaseColorG->SetValue(Param.BaseColor.G);
    SliderBaseColorB->SetValue(Param.BaseColor.B);
    InputBaseColorR->SetText(FText::FromString(FString::FromInt(Param.BaseColor.R * 255.0f)));
    InputBaseColorG->SetText(FText::FromString(FString::FromInt(Param.BaseColor.G * 255.0f)));
    InputBaseColorB->SetText(FText::FromString(FString::FromInt(Param.BaseColor.B * 255.0f)));

    SliderSpecularColorR->SetValue(Param.SpecularColor.R);
    InputSpecularColorR->SetText(FText::FromString(FString::FromInt(Param.SpecularColor.R * 255.0f)));

    SliderEmissiveColorR->SetValue(Param.EmissiveColor.R);
    SliderEmissiveColorG->SetValue(Param.EmissiveColor.G);
    SliderEmissiveColorB->SetValue(Param.EmissiveColor.B);
    InputEmissiveColorR->SetText(FText::FromString(FString::FromInt(Param.EmissiveColor.R * 255.0f)));
    InputEmissiveColorG->SetText(FText::FromString(FString::FromInt(Param.EmissiveColor.G * 255.0f)));
    InputEmissiveColorB->SetText(FText::FromString(FString::FromInt(Param.EmissiveColor.B * 255.0f)));

    SliderShininess->SetValue(Param.Shininess);
    InputShininess->SetText(FText::FromString(FString::FromInt(Param.Shininess * 255.0f)));

    SliderTransparency->SetValue(Param.Transparency);
    InputTransparency->SetText(FText::FromString(FString::FromInt(Param.Transparency * 255.0f)));

    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderBaseColorR(float Value) {
    InputBaseColorR->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderBaseColorG(float Value) {
    InputBaseColorG->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderBaseColorB(float Value) {
    InputBaseColorB->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderSpecularColorR(float Value) {
    InputSpecularColorR->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderEmissiveColorR(float Value) {
    InputEmissiveColorR->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderEmissiveColorG(float Value) {
    InputEmissiveColorG->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderEmissiveColorB(float Value) {
    InputEmissiveColorB->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderShininess(float Value) {
    InputShininess->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnValueChangedSliderTransparency(float Value) {
    InputTransparency->SetText(FText::FromString(FString::FromInt(Value * 255.0f)));
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputBaseColorR(const FText& Text) {
    SliderBaseColorR->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputBaseColorG(const FText& Text) {
    SliderBaseColorG->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputBaseColorB(const FText& Text) {
    SliderBaseColorB->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputSpecularColorR(const FText& Text) {
    SliderSpecularColorR->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputEmissiveColorR(const FText& Text) {
    SliderEmissiveColorR->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputEmissiveColorG(const FText& Text) {
    SliderEmissiveColorG->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputEmissiveColorB(const FText& Text) {
    SliderEmissiveColorB->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputShininess(const FText& Text) {
    SliderShininess->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnTextChangedInputTransparency(const FText& Text) {
    SliderTransparency->SetValue(FCString::Atof(*Text.ToString()) / 255.0f);
    OnRefrectMaterial();
}

void UTwinLinkColorAdjustmentEdit::OnClickedClose() {
    RefrectParameter(RevertMaterialParameter);
    this->SetVisibility(ESlateVisibility::Hidden);
}

void UTwinLinkColorAdjustmentEdit::OnClickedReset() {
    auto ColorAdjustmentSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkColorAdjustmentSystem>();
    check(ColorAdjustmentSystem.IsValid());
    RefrectParameter(ColorAdjustmentSystem.Get()->MaterialParameterOrigin[MaterialIndex]);
    ColorAdjustmentSystem.Get()->ExportColorAdjustmentParam();
    this->SetVisibility(ESlateVisibility::Hidden);

    Thumbnail->SetColorAndOpacity(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].BaseColor);
}

void UTwinLinkColorAdjustmentEdit::OnClickedRegist() {
    auto ColorAdjustmentSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkColorAdjustmentSystem>();
    check(ColorAdjustmentSystem.IsValid());
    ColorAdjustmentSystem.Get()->ExportColorAdjustmentParam();
    this->SetVisibility(ESlateVisibility::Hidden);

    Thumbnail->SetColorAndOpacity(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].BaseColor);
}

void UTwinLinkColorAdjustmentEdit::OnRefrectMaterial() {
    auto ColorAdjustmentSystem = TwinLinkSubSystemHelper::GetInstance<UTwinLinkColorAdjustmentSystem>();
    check(ColorAdjustmentSystem.IsValid());
    auto Parameter = ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex];
    auto Refrect = ColorAdjustmentSystem.Get()->Materials[MaterialIndex];

    FLinearColor BaseColor = FLinearColor(SliderBaseColorR->GetValue(), SliderBaseColorG->GetValue(), SliderBaseColorB->GetValue());
    ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].BaseColor = BaseColor;
    Cast<UMaterialInstanceDynamic>(Refrect)->SetVectorParameterValue(FName(TEXT("BaseColor")), BaseColor);

    FLinearColor SpecularColor = FLinearColor(SliderSpecularColorR->GetValue(), ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].SpecularColor.G, ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].SpecularColor.B);
    ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].SpecularColor = SpecularColor;
    Cast<UMaterialInstanceDynamic>(Refrect)->SetVectorParameterValue(FName(TEXT("SpecularColor")), SpecularColor);

    FLinearColor EmissiveColor = FLinearColor(SliderEmissiveColorR->GetValue(), SliderEmissiveColorG->GetValue(), SliderEmissiveColorB->GetValue());
    ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].EmissiveColor = EmissiveColor;
    Cast<UMaterialInstanceDynamic>(Refrect)->SetVectorParameterValue(FName(TEXT("EmissiveColor")), EmissiveColor * ColorAdjustmentSystem.Get()->EmissibeStrength);

    float Shininess = SliderShininess->GetValue();
    ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].Shininess = Shininess;
    Cast<UMaterialInstanceDynamic>(Refrect)->SetScalarParameterValue(FName(TEXT("Shininess")), Shininess);

    float Transparency = SliderTransparency->GetValue();
    ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].Transparency = Transparency;
    Cast<UMaterialInstanceDynamic>(Refrect)->SetScalarParameterValue(FName(TEXT("Transparency")), Transparency);

    MaterialPreview->SetColorAndOpacity(ColorAdjustmentSystem.Get()->MaterialParameter[MaterialIndex].BaseColor);
}