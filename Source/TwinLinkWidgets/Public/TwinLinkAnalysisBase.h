// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkTabContentBase.h"
#include "TwinLinkAnalysisBase.generated.h"

/**
 * 解析
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkAnalysisBase : public UTwinLinkTabContentBase
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

protected:
    /**
     * @brief 有効化された時に呼び出される関数
    */
    virtual void OnActivated() override;

    /**
     * @brief 無効化された時に呼び出される関数
    */
    virtual void OnDeactivate() override {};

private:
    /**
     * @brief 初期化
    */
    void Setup();

    /**
     * @brief 破棄処理
    */
    void Finalize();

};
