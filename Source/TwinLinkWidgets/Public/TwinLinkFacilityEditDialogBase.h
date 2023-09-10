// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkFacilityEditDialogBase.generated.h"

// 施設情報
class UTwinLinkFacilityInfo;

/**
 * 施設情報編集用ダイアログの基底クラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFacilityEditDialogBase : public UTwinLinkWidgetBase {
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
     * @brief カテゴリの表示名を取得する
     *
     * memo 本来はWidget(BP)層で表示カテゴリ名を保持して変換をWidget(C++)層で行うべき
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetDisplayCategoryName() const;

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
        const FString& Name,
        const FString& Category,
        const FString& ImageFileName,
        const FString& Guide,
        const FString& SpotInfo);

    /**
     * @brief 編集のリクエストに成功した時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnSuccessRequestEdit();

    /**
     * @brief 編集のリクエストに失敗した時に呼ばれる
     *
     * memo 失敗原因をUI側に表示するなら引数に情報を含めてもいいかも
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnFailedRequestEdit();


    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedInfo();

    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedCategoryGroup(const TArray<FString>& Categories);


private:
    TWeakObjectPtr<UTwinLinkFacilityInfo> FacilityInfo;
    FDelegateHandle EvOnChangedHnd;
};
