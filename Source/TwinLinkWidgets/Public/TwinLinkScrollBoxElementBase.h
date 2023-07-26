// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TwinLinkScrollBoxElementBase.generated.h"

/**
 * 視点情報リストの要素のウィジェットに継承させるクラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkScrollBoxElementBase : public UUserWidget
{
	GENERATED_BODY()
public:
    /**
     * @brief データに基づいて設定を行う
     * @param Obj 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UObject* Obj);

    /**
     * @brief この要素に合わせて同期する
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void Sync();

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
    void RequestEditName(const FString NewName) const;

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


protected:
    // 継承先でbriefの機能を実装してください

    /**
     * @brief この要素の削除要求を行う
     * @param Data 
    */
    virtual void OnRequestRemove(UObject* Data) const {};

    /**
     * @brief この要素の編集要求を行う
    */
    virtual void OnRequestEdit(const FString NewName) const {};

    /**
     * @brief 要素の名前を取得する
     * @param Data 
     * @return 
    */
    virtual FText OnGetName(UObject* Data) const { return FText(); };

    /**
     * @brief この要素を選択する要求を行う
     * @param Data 
    */
    virtual void OnRequestSelect(UObject* Data) const {};

private:
    /** このウィジェットが示すデータ **/
    TWeakObjectPtr<UObject> WidgetData;

};
