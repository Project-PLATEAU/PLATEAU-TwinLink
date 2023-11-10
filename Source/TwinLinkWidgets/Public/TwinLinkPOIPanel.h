// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "TwinLinkPOIPanel.generated.h"

/**
 * GISデータ表示ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkPOIPanel : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**
    * @brief シェイプファイル読み込み
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkGISDataGetSource(const TArray<FString>& Files);
    /**
    * @brief 属性を登録
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkPOIRegist();
    /**
    * @brief 初期化
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkPOIClear();
    /**
    * @brief 初期化
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkPOISetActive(bool IsShow);
public:
    /** 属性選択 **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UComboBoxString> AttributeCombobox;
    /** 属性テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UEditableTextBox> AttributeText;
    /** 属性テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UScrollBox> RegisteredAttributesScrollbox;
private:
    /**
    * @brief POI登録
    */
    void RegistPOI(const FString& RegisteredName);
    /**
    * @brief POI表示
    */
    void ShowPOI(const FString& RegisteredName);
};
