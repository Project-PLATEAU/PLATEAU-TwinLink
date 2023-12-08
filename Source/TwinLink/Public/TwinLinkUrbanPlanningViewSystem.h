// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "Engine/DataTable.h"
#include "TwinLinkUrbanPlanningDataAsset.h"
#include "TwinLinkUrbanPlanningViewSystem.generated.h"

/**
 * ライン情報
 */
USTRUCT()
struct FTwinLinkEdge : public FTableRowBase {
    GENERATED_BODY()
public:
    /** 開始座標 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector StartPoint;
    /** 終端座標 **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector EndPoint;
    /**
     * @brief デフォルトコンストラクタ
    */
    FTwinLinkEdge() { StartPoint = FVector(); EndPoint = FVector(); }
    /**
     * @brief コンストラクタ
     * @param Start
     * @param End
    */
    FTwinLinkEdge(const FVector& Start, const FVector& End) {
        StartPoint = Start;
        EndPoint = End;
    }
};

/**
 * 都市計画決定情報の可視化を行うサブシステム
 */
UCLASS()
class TWINLINK_API UTwinLinkUrbanPlanningViewSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()
public:
    /**
     * @brief 都市計画決定情報の表示を開始
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkShowUrbanPlanning();
    /**
     * @brief 都市計画決定情報の表示を終了
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkHideUrbanPlanning();
    /**
     * @brief 都市計画決定情報の表示を選択
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void TwinLinkSelectUrbanPlanning();
    /**
     * @brief 容積率を取得
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        FString TwinLinkGetUrbanPlanningRate();
    /**
     * @brief 用途を取得
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        FString TwinLinkGetUrbanPlanningUsage();
    /**
     * @brief 用途色マップを取得
    */
    TMap<FString, FColor> GetGuideUsageColors();
    /**
     * @brief 用途定義を取得
    */
    TObjectPtr<UTwinLinkUrbanPlanningDataAsset> GetFunctionDefines() { return UrbanPlanningFunctionDefines; }
protected:
    /** マテリアルへ適用するカラー **/
    UPROPERTY()
        TArray<FColor> OriginColors {
        FColor::FromHex("#1ABC9CFF"),  // ターコイズ
            FColor::FromHex("#3498DAFF"),  // ブルー
            FColor::FromHex("#9C58B7FF"),  // パープル
            FColor::FromHex("#E74C3CFF"),  // レッド
            FColor::FromHex("#E77F22FF")   // オレンジ
    };
private:
    /** 使用するマテリアルインスタンス群 **/
    UPROPERTY()
        TMap<FString, TObjectPtr<UMaterialInstanceDynamic>> MaterialCollection;
    /** 使用するマテリアルインスタンス群 **/
    UPROPERTY()
        TMap<FString, TObjectPtr<UMaterialInstanceDynamic>> MaterialCollectionPlane;
    /** 表示用のアクタ **/
    UPROPERTY()
        TObjectPtr<AActor> ViewActor;
    /** 属性情報インデックス **/
    UPROPERTY()
        TMap<TObjectPtr<USceneComponent>, int> AttributeIndexCollection;
    /** 容積率 **/
    TArray<FString> AttributeRates;
    /** 用途 **/
    TArray<FString> AttributeUsages;
    /** 選択した属性情報のインデックス **/
    int SelectAttributesIndex = -1;
    /** 凡例用途カラー **/
    TMap<FString, FColor> GuideUsageColors;
    /** 用途カラー定義 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkUrbanPlanningDataAsset> UrbanPlanningFunctionDefines;
private:
    /**
     * @brief マテリアルインスタンスを生成する
    */
    void AddMaterials(const FString& NewUsage);
    /**
     * @brief 重複ラインを削除する
    */
    TArray<FTwinLinkEdge> RemoveOverlapLinePath(TMap<FString, int> Maps, TArray<FVector>Points);
    /**
     * @brief 都市計画決定情報の定義をロードする
    */
    void LoadUrbanPlanningFunctionDefine();
    /**
     * @brief 都市計画決定情報の定義をロードする
    */
    FTwinLinkUrbanPlanningFunction* GetUrbanPlanningFunction(const FString& NewUsage);
};
