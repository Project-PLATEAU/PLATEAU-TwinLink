// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkObservableDataObjBase.h"

#include "TwinLinkName3DData.h"
#include "TwinLinkFloorInfo.generated.h"

/**
 * フロアに含まれる施設内店舗の情報を保持するクラス
 * データ数が変化した場合は
 */
UCLASS(BlueprintType, BlueprintType)
class TWINLINK_API UTwinLinkFloorInfo : public UTwinLinkObservableDataObjBase, public ITwinLinkName3DData
{
	GENERATED_BODY()
	
public:
    /**
     * @brief 正常な値がチェックする
     * @param InID 
     * @param InName 
     * @param InGuideText 
     * @param InImageFileName 
     * @return 正常ならtrue
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    static bool IsValid(
        const FString& InName,
        const FString& InCategory,
        const FVector& InLocation,
        const FString& InImageFileName,
        const FString& InGuideText,
        const FString& InOpningHoursText);

public:
    
    void Setup(
        const FString& InName,
        const FString& InCategory,
        const FVector& InLocation,
        const FString& InImageFileName,
        const FString& InGuideText, 
        const FString& InOpningHoursText);

    /**
     * @brief 初期化
     * @param DataStr データの文字列Collection 
    */
    void Setup(TArray<FString> DataStr);

    /**
     * @brief 施設内店舗名の取得
     * ITwinLinkName3DData継承
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    virtual FString GetName() const override { return Name; }

    /**
     * @brief カテゴリの取得
     * @attention 表示用文字列ではないので注意
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetCategory() const { return Category; }

    /**
     * @brief 座標の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FVector GetLocation() const { return Location; }

    /**
     * @brief 座標の取得
     * ITwinLinkName3DData継承
     * @return
    */
    virtual FVector GetPosition() const override { return Location; }

    /**
     * @brief 施設内店舗のイメージ名の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetImageFileName() const { return ImageFileName; }

    /**
     * @brief 案内情報の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetGuideText() const { return GuideText; }

    /**
     * @brief 営業時間の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetOpningHoursText() const { return OpningHoursText; }

private:
    /** データ数 **/ 
    static const int NumDataElement = 8; // FVectorは3として数える

    /** 施設名 **/
    FString Name;

    /** カテゴリ **/
    FString Category;

    /** Location **/
    FVector Location;

    /** 拡張子付きのイメージのファイル名 **/
    FString ImageFileName;

    /** 案内表示テキスト **/
    FString GuideText;

    /** スポット情報表示テキスト **/
    FString OpningHoursText;

};
