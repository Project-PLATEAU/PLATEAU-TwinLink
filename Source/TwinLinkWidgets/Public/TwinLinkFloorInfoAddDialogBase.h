// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkFloorInfoAddDialogBase.generated.h"

/**
 * 店舗情報追加ダイアログのウィジェットの基底クラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFloorInfoAddDialogBase : public UTwinLinkWidgetBase
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
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(const FVector& InLocationVector);

    /**
     * @brief 追加可能な情報かチェックする
     * @param Name
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    bool CheckAddableFloorInfo(
        const FString& InName,
        const FString& InCategory,
        const FString& InImageFileName,
        const FString& InGuideText,
        const FString& InOpningHoursText);

    /**
     * @brief 情報を追加する
     * @param InName
     * @param InCategory
     * @param InImageFileName
     * @param InGuideText
     * @param InOpningHoursText
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void AddFloorInfo(
        const FString& InName,
        const FString& InCategory,
        const FString& InImageFileName,
        const FString& InGuideText,
        const FString& InOpningHoursText);

    /**
     * @brief カテゴリの種類を設定する
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void SyncCategoryCollectionWidget(const TArray<FString>& Categories);

private:
    FVector Location;
};
