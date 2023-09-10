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
     * @param CityModel
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void SetupTwinLinkFloorView();

    /**
     * @brief 階層表示パネル要素選択時処理
     * @param Element
    */
    void FloorViewChange(TObjectPtr<UUserWidget> Element);

public:
    /** 階層表示パネル要素追加先パネルウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UPanelWidget> FloorViewScrollBox;

private:
    /** 都市モデルの階層コンポーネントのコレクション **/
    TMap<FString, TObjectPtr<UPLATEAUCityObjectGroup>> LinkComponents;

    /** 階層表示パネル要素のコレクション **/
    TMap<FString, TObjectPtr<UUserWidget>> ElementWidgets;

    /** 階層表示パネル要素のソート済みキー配列 **/
    TArray<FString> FloorKeys;
};
