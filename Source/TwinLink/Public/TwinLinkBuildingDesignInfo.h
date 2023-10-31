// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkObservableDataObjBase.h"
#include "TwinLinkBuildingDesignInfo.generated.h"

/**
 * 設計情報を保持したデータクラス
 */
UCLASS(BlueprintType, BlueprintType)
class TWINLINK_API UTwinLinkBuildingDesignInfo : public UTwinLinkObservableDataObjBase
{
	GENERATED_BODY()

public:
    /**
     * @brief 正常な値がチェックする
     * @param InImageFileName
     * @return 正常ならtrue
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    static bool IsValid(
        const FString& InImageFileName);

public:
    /**
     * @brief 初期化
     * @param InImageFileName 
    */
    void Setup(
        const FString& InImageFileName);

    /**
     * @brief 初期化
     * @param DataStr データの文字列Collection
    */
    void Setup(TArray<FString> DataStr);

    /**
     * @brief 施設内店舗のイメージ名の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetImageFileName() const { return ImageFileName; }

private:
    /** データ数 **/
    static const int NumDataElement = 1; // FVectorは3として数える

    /** 拡張子付きのイメージのファイル名 **/
    FString ImageFileName;

};
