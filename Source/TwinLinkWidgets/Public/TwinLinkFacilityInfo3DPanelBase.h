// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLink3DUMGBase.h"
#include "TwinLinkFacilityInfo3DPanelBase.generated.h"

// 施設情報
class UTwinLinkFacilityInfo;

/**
 *
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFacilityInfo3DPanelBase : public UTwinLink3DUMGBase {
    GENERATED_BODY()

public:
    /**
     * @brief ウィジェットとデータの連携
     * @param Info 利用する施設情報
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UObject* Info, float MinimumHeightToDisplay);

    /**
     * @brief 施設情報の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    UTwinLinkFacilityInfo* GetData() const {
        check(FacilityInfo.IsValid());
        return FacilityInfo.Get();
    }

    /**
     * @brief 表示位置
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FVector GetDisplayPosition() const;

    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedInfo();

private:
    TWeakObjectPtr<UTwinLinkFacilityInfo> FacilityInfo;

    /** 表示時の高さ **/
    FVector DisplayPosition;
};
