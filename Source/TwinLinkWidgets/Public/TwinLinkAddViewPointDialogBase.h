// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TwinLinkAddViewPointDialogBase.generated.h"

/**
 * 視点情報を追加を扱うダイアログの基底クラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkAddViewPointDialogBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
    /**
     * @brief 視点情報を追加する
     * @param ViewPointName 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void AddViewPointInfo(const FString ViewPointName);

    /**
     * @brief 視点情報が追加されたときに呼び出される
     * @param ViewPointName 
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnAddedViewPointInfo(const FString& ViewPointName);

    /**
     * @brief 視点情報の追加に失敗したときに呼び出される
     * @param ViewPointName 
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnFailedAddViewPointInfo(const FString& ViewPointName);

private:

};
