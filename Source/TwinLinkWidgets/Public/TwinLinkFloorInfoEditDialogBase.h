// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkFloorInfoEditDialogBase.generated.h"

class UTwinLinkFloorInfo;

/**
 * 店舗情報の編集ダイアログウィジェットの基底クラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFloorInfoEditDialogBase : public UTwinLinkWidgetBase
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
     * @brief ウィジェットとデータの連携
     * @param Info 利用する施設情報
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UTwinLinkFloorInfo* Info);

    /**
     * @brief 施設情報の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    UTwinLinkFloorInfo* GetData() const;

    /**
     * @brief カテゴリの表示名を取得する
     *
     * memo 本来はWidget(BP)層で表示カテゴリ名を保持して変換をWidget(C++)層で行うべき
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetDisplayCategoryName() const;

    /**
     * @brief カテゴリグループ一覧を取得する
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    TArray<FString> GetCategoryGroup() const;

    /**
     * @brief 施設情報の変更リクエストを行う
     * @param Name
     * @param _Category
        UFUNCTION(Category)と被るためアンダースコア付き
     * @param ImageFileName
     * @param Guide
     * @param SpotInfo
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestEdit(
        const FString& InName,
        const FString& InCategory,
        const FString& InImageFileName,
        const FString& InGuide,
        const FString& InSpotInfo);

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    bool CheckAddableFloorInfo(
        const FString& InName,
        const FString& InCategory,
        const FString& InImageFileName,
        const FString& InGuideText,
        const FString& InOpningHoursText);

    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedInfo();

private:
    TWeakObjectPtr<UTwinLinkFloorInfo> FloorInfo;
    FDelegateHandle EvOnChangedHnd;
};
