// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkScrollBoxElementBase.generated.h"

// 実装
class UTwinLinkScrollBoxElementImpl;

/**
 * 情報リストの要素のウィジェットに継承させるクラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkScrollBoxElementBase : public UTwinLinkWidgetBase {
    GENERATED_BODY()
public:
    /**
     * @brief データに基づいて設定を行う
     * @param Obj
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UTwinLinkScrollBoxElementImpl* _Impl, UObject* Obj);

    /**
     * @brief この要素に合わせて同期する
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedElement();

    /**
     * @brief この要素の削除を要求する
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestRemove() const;

    /**
     * @brief この要素の名前を変更を要求する
     * @param NewName
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestEditName(const FString& NewName) const;

    /**
     * @brief 内部データを取得する
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    const UObject* GetElement() const;

    /**
     * @brief 名前を取得する
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    const FText GetName() const;

    /**
     * @brief この要素を選択する
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestSelect() const;

private:
    UPROPERTY()
    TWeakObjectPtr<UTwinLinkScrollBoxElementImpl> Impl;

    /** このウィジェットが示すデータ **/
    TWeakObjectPtr<UObject> WidgetData;

    FDelegateHandle EvOnChangedHnd;

};
