// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkMainWindowBase.generated.h"

/**
 *
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkMainWindowBase : public UTwinLinkWidgetBase {
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
