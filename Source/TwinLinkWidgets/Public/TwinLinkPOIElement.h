// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TwinLinkPOIElement.generated.h"

/**
 * POI選択要素ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkPOIElement : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**
    * @brief 登録削除
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkPOIRemove();
    /**
    * @brief 表示切替
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkPOIVisible();
public:
    /** 属性テキスト **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> AttributeText;
    /** ピンアイコン **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UImage> AttributeImage;
    /** カラー割り当て **/
    FColor Color;
};
