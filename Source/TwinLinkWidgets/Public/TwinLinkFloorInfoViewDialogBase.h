// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkFloorInfoViewDialogBase.generated.h"

// データ
class UTwinLinkFloorInfo;

/**
 * 店舗情報閲覧用のダイアログ
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFloorInfoViewDialogBase : public UTwinLinkWidgetBase
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
     * @brief データの設定
     * @param InData
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UObject* InData);

    /**
     * @brief 店舗情報の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    UTwinLinkFloorInfo* GetData() const;

    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedInfo();

private:
    TWeakObjectPtr<UTwinLinkFloorInfo> Data;
    FDelegateHandle EvOnChangedHnd;
};
