// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkFacilityInfoViewBase.generated.h"

/**
 *
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFacilityInfoViewBase : public UTwinLinkWidgetBase {
    GENERATED_BODY()

public:
    /**
     * @brief ウィジェットとデータの連携
     * @param Info 利用する施設情報
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UTwinLinkFacilityInfo* Info);

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
     * @brief イメージファイルのパスを作成する
     * @param InFileName 拡張子付きファイル名
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString CreateImageFilePath(const FString& InFileName);

    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedInfo();

private:
    TWeakObjectPtr<UTwinLinkFacilityInfo> FacilityInfo;
    FDelegateHandle EvOnChangedHnd;
};
