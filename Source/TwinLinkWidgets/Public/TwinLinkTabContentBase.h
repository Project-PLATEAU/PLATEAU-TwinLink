// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkTabContentBase.generated.h"

/**
 * タブのコンテンツの基底ウィジェット
 * タブ切替時に有効化、無効化されることを想定している
 * 
 * Widget(CPP)層で有効化、無効化時に処理を実行したい場合は
 * OnActivated(), OnDeactivate()をオーバーライドすること
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkTabContentBase : public UTwinLinkWidgetBase
{
	GENERATED_BODY()

public:
    /**
     * @brief コンストラクト
    */
    virtual void NativeConstruct() override;

    /**
     * @brief デストラクト
    */
    virtual void NativeDestruct() override;

    /**
     * @brief ウィジェットの有効化、無効化を制御する
     * 状態が切り替わった時に内部で対応するイベントやコールバックが呼び出される
     * 呼び出し順はWidget(C++), Widget(BP)。内部データの変更を優先するため。
     * 
     * @param InbIsActive
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void SetActive(bool bInIsActive);

protected:
    /**
     * @brief 有効化された時に呼び出される関数
     * 継承先でオーバーライドして拡張すること
    */
    virtual void OnActivated() {};

    /**
     * @brief 無効化された時に呼び出される関数
     * 継承先でオーバーライドして拡張すること
    */
    virtual void OnDeactivate() {};

protected:
    /**
     * @brief 有効、無効の状態が切り替わった
     * @param InbIsActive 
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedActivity(bool bInIsActive);

private:
    bool bIsActive;
};
