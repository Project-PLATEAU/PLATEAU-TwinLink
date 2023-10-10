// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TwinLinkWidgetSetup.generated.h"


class UTwinLinkScrollBoxBase;
class UTwinLinkScrollBoxElementImpl;
class UTwinLink3DUMGCanvasBase;
class UTwinLinkObservableCollection;

/**
 * 基底ウィジェットの設定クラス
 * 設定を行う関数が配置されている
 *
 * 増えてきたら整備予定
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkWidgetSetup : public UUserWidget {
    GENERATED_BODY()

public:
    /**
     * @brief 視点情報のScrollboxウィジェットのセットアップを行う
     * @param Widget 
     * @param ElementImpl 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink Widget Setup")
    static void SetupViewPointList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl);

    /**
     * @brief 施設情報のScrollboxウィジェットのセットアップを行う
     * @param Widget 
     * @param ElementImpl 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink Widget Setup")
    static void SetupFacilityInfoList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl);

    /**
     * @brief フロア情報のScrollboxウィジェットのセットアップを行う
     * @param Widget 
     * @param ElementImpl 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink Widget Setup")
    static void SetupFloorInfoList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl);

    /**
     * @brief 選択しているフロア情報のコレクションを取得する
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink Widget Setup")
    static UTwinLinkObservableCollection* GetSelectedFloorInfoCollection();

};
