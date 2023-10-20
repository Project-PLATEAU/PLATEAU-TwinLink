// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "TwinLinkColorAdjustmentSystem.h"
#include "TwinLinkColorAdjustmentPreview.h"
#include "TwinLinkColorAdjustmentEdit.generated.h"

/**
 * マテリアル編集ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkColorAdjustmentEdit : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**  プレビュー  **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UImage> MaterialPreview;

    /** マテリアル名 **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> TextName;

    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderBaseColorR;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputBaseColorR;
    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderBaseColorG;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputBaseColorG;
    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderBaseColorB;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputBaseColorB;

    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderSpecularColorR;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputSpecularColorR;

    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderEmissiveColorR;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputEmissiveColorR;
    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderEmissiveColorG;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputEmissiveColorG;
    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderEmissiveColorB;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputEmissiveColorB;

    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderShininess;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputShininess;

    /** スライダー **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<USlider> SliderTransparency;
    /** テキストボックス **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> InputTransparency;

    /** リセットボタン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> CloseButton;

    /** リセットボタン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> ResetButton;

    /** 登録ボタン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> RegistButton;

    /** マテリアル参照インデックス **/
    int MaterialIndex;

    /** 保存しなかった場合に戻す際のマテリアル情報 */
    FTwinLinkAdjustmentMaterialParameter RevertMaterialParameter;

    /** マテリアル選択ウィジェットのサムネイル **/
    TWeakObjectPtr<UImage> Thumbnail;
public:
    /**
     * @brief 編集ウィジェット初期化
    */
    void SetupEditor(const int Index);
    /**
     * @brief パラメータ反映
    */
    void RefrectParameter(const FTwinLinkAdjustmentMaterialParameter& Param);
    /**
     * @brief 要素操作時コールバック
    */
    UFUNCTION()
        void OnValueChangedSliderBaseColorR(float Value);
    UFUNCTION()
        void OnValueChangedSliderBaseColorG(float Value);
    UFUNCTION()
        void OnValueChangedSliderBaseColorB(float Value);
    UFUNCTION()
        void OnValueChangedSliderSpecularColorR(float Value);
    UFUNCTION()
        void OnValueChangedSliderEmissiveColorR(float Value);
    UFUNCTION()
        void OnValueChangedSliderEmissiveColorG(float Value);
    UFUNCTION()
        void OnValueChangedSliderEmissiveColorB(float Value);
    UFUNCTION()
        void OnValueChangedSliderShininess(float Value);
    UFUNCTION()
        void OnValueChangedSliderTransparency(float Value);

    UFUNCTION()
        void OnTextChangedInputBaseColorR(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputBaseColorG(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputBaseColorB(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputSpecularColorR(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputEmissiveColorR(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputEmissiveColorG(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputEmissiveColorB(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputShininess(const FText& Text);
    UFUNCTION()
        void OnTextChangedInputTransparency(const FText& Text);
    UFUNCTION()
        void OnClickedClose();
    UFUNCTION()
        void OnClickedReset();
    UFUNCTION()
        void OnClickedRegist();
private:
    /**
     * @brief マテリアル反映
    */
    void OnRefrectMaterial();
};
