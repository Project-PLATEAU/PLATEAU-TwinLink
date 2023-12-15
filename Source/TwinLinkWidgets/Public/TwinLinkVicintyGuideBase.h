// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkTabContentBase.h"
#include "TwinLinkVicintyGuideBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkVicintyGuideBase : public UTwinLinkTabContentBase
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
    /** 施設をクリックした時のイベントHandle **/
    FDelegateHandle EvOnClickedFacilityHnd;

protected:
    /**
     * @brief 施設が選択された時に呼ばれる
     * @param FeatureID 地物ID
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnClickedFacility(const FString& FeatureID);

    /**
     * @brief 仮で作成 WorldViewerを俯瞰視点に戻す
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void ResetWorldViewerStateToOverLook();

private:
    /**
     * @brief 管理者モード関係の初期設定
    */
    void SetupMainWindow();

    /**
     * @brief 管理者モード関係の終了時処理
    */
    void FinalizeMainWindow();
    
};
