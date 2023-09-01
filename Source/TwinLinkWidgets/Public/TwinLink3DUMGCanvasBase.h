// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLink3DUMGCanvasBase.generated.h"

// ScrollBoxの要素群を保持するCollection
class UTwinLinkObservableCollection;

/**
 *
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLink3DUMGCanvasBase : public UTwinLinkWidgetBase {
    GENERATED_BODY()

public:
    /**
     * @brief ウィジェットとデータの連携
     * @param Info 利用する施設情報
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UTwinLinkObservableCollection* Collection);

    /**
     * @brief 要素が追加されたときに呼び出される
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnPostAddElement(const UObject* Element);

    /**
     * @brief 要素が削除されたときに呼び出される
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnPostRemoveElement(const UObject* Element);

    /**
     * @brief 全要素がクリアされたときに呼び出される
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnPostReset();

private:
    /**
     * @brief システム層が保持するコレクションへの参照
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> ObservableCollection;

    ///**
    // * @brief 管理ウィジェット検索用マップ
    //*/
    //TMap<int, UTwinLinkScrollBoxElementBase*> ScrollBoxElementWidgets;

};
