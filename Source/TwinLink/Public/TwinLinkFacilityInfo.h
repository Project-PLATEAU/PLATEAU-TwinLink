// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkObservableDataObjBase.h"
#include "TwinLinkFacilityInfo.generated.h"

// 施設情報変更時のイベント
DECLARE_EVENT(UTwinLinkFacilityInfo, FDelOnChanged);

/**
 * 施設情報を保持するクラス
 */
UCLASS(BlueprintType)
class TWINLINK_API UTwinLinkFacilityInfo : public UTwinLinkObservableDataObjBase {
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
        const FString& InDescription, const FString& InSpotInfo, const TArray<FVector>& Entrance);

    /**
     * @brief 初期化
     * @param DataStr
     * @return
    */
    bool Setup(const TArray<FString>& DataStr);

    /*
     * @brief : 入口情報を設定
     */
    void SetEntrances(const TArray<FVector>& Value);

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

    /**
     * @brief 建物の入口座標を取得(経路探索用)
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        const TArray<FVector>& GetEntrances() const { return Entrances; }

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        bool TryGetFirstEntrance(FVector& Out) const;

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

    /** 建物の入口(ワールド座標). 空だと設定なし **/
    TArray<FVector> Entrances;
};
