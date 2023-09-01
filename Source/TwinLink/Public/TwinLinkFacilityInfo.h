// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkDataObjBase.h"
#include "TwinLinkFacilityInfo.generated.h"

// 施設情報変更時のイベント
DECLARE_EVENT(UTwinLinkFacilityInfo, FDelOnChanged);

/**
 * 施設情報を保持するクラス
 */
UCLASS(BlueprintType, BlueprintType)
class TWINLINK_API UTwinLinkFacilityInfo : public UTwinLinkDataObjBase {
    GENERATED_BODY()

public:
    /**
     * @brief 初期化
     * @param _BuildingGuide
     * @param _SpotInfo
     * @return
    */
    bool Setup(
        const FString& InName, const FString& InCategory,
        const FString& InFeatureID, const FString& InImageFileName,
        const FString& InDescription, const FString& InSpotInfo);

    /**
     * @brief 初期化
     * @param DataStr
     * @return
    */
    bool Setup(const TArray<FString>& DataStr);

    /**
     * @brief 施設名の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetName() const { return Name; }

    /**
     * @brief 施設カテゴリの取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetCategory() const { return Category; }

    /**
     * @brief 地物IDの取得
     * 地物IDはPLATEAUによってインポートされたモデルのスタティックメッシュコンポーネントの名前
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetFeatureID() const { return FeatureID; }

    /**
     * @brief 施設イメージ名の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetImageFileName() const { return ImageFileName; }

    /**
     * @brief 施設詳細の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetDescription() const { return Description; }

    /**
     * @brief スポット情報の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetSpotInfo() const { return SpotInfo; }

public:
    /** データが変更された **/
    FDelOnChanged EvOnChanged;

private:
    /** 施設名 **/
    FString Name;

    /** カテゴリ enumに変更予定 **/
    FString Category;

    /** 地物ID　建物をクリックした時に選択されるコンポーネント名 **/
    FString FeatureID;

    /** 拡張子付きのイメージのファイル名 **/
    FString ImageFileName;

    /** 詳細 **/
    FString Description;

    /** スポット情報 **/
    FString SpotInfo;
};
