// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "PLATEAUInstancedCityModel.h"
#include "Containers/SortedMap.h"
#include "TwinLinkFloorViewPanel.generated.h"

/**
 *　階層表示パネルウィジェット
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFloorViewPanel : public UTwinLinkWidgetBase {
    GENERATED_BODY()

public:
    /**
     * @brief 階層表示パネルセットアップ
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void SetupTwinLinkFloorView();

    /**
     * @brief 階層表示パネル後始末
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void FinalizeTwinLinkFloorView();

    /**
     * @brief 階層表示パネル要素選択時処理
     * @param Element
    */
    void FloorViewChange(TObjectPtr<UUserWidget> Element);

    /**
     * @brief 選択している階層が変更された時に呼ばれる
     * （ただし現状は同じ階層を選択しても呼ばれる）
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedFloorView();

public:
    /** 階層表示パネル要素追加先パネルウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UPanelWidget> FloorViewScrollBox;

private:
    /** 都市モデルの階層コンポーネントのコレクション **/
    TMap<FString, TWeakObjectPtr<UPLATEAUCityObjectGroup>> LinkComponents;

    /** 階層表示パネル要素のコレクション **/
    TMap<FString, TObjectPtr<UUserWidget>> ElementWidgets;

    /** 階層表示パネル要素のソート済みキー配列 **/
    TArray<FString> FloorKeys;
};
