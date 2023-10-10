// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkInfo3DPanelBase.generated.h"

// 通知機能付きコレクション
class UTwinLinkObservableDataObjBase;

// 要求するデータのインターフェイス
class ITwinLinkName3DData;

/**
 * 3D空間上に情報を配置するウィジェットの基底クラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkInfo3DPanelBase : public UTwinLinkWidgetBase
{
	GENERATED_BODY()
	
public:
    /**
     * @brief データのセットアップ
     * ウィジェットの生成直後、ウィジェットの初期化前に呼ぶ
     * @param Info 利用するデータ
     * @param Offset 表示位置
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UObject* Info, FVector Offset);

    /**
     * @brief 情報の取得
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetInfo() const;

    /**
     * @brief 表示位置を取得する
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FVector GetPosition() const;

    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedInfo();

private:
    /** 通知機能付きのデータ **/
    TWeakObjectPtr<UTwinLinkObservableDataObjBase> InfoObservable;
    /** データ取得のインターフェイス **/
    ITwinLinkName3DData* InfoGetter;

    /** 表示位置 **/
    FVector DisplayOffset;

};
