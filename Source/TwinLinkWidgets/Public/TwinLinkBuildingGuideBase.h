// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkTabContentBase.h"
#include "TwinLinkBuildingGuideBase.generated.h"

/**
 * 建物案内
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkBuildingGuideBase : public UTwinLinkTabContentBase
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

public:
    /**
     * @brief 表示しているGrpIDと一致しているかチェックする
     * ライントレースで選択した階層が表示されている階層と一致しているかの確認で使っている
     * @param InGrpID 
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    bool CheckIsSelectedGrpID(const FString& InGrpID);

protected:
    /**
     * @brief 有効化された時に呼び出される関数
    */
    virtual void OnActivated() override {};

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
