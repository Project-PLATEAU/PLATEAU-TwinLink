// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "TwinLinkColorAdjustmentElement.generated.h"

class UTwinLinkColorAdjustmentPanel;

/**
 * マテリアル選択要素ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkColorAdjustmentElement : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**  プレビュー  **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UImage> MaterialPreview;
    /** マテリアル名 **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> MaterialName;
    /** 編集ボタン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> MaterialEdit;
    /** 選択時通知先ウィジェットへの参照 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkColorAdjustmentPanel> ElementRoot;
    /** マテリアルコレクションインデックス **/
    int MaterialIndex;
public:
    /**
     * @brief 編集ボタン選択時
    */
    UFUNCTION()
        void OnSelectEdit();
};
