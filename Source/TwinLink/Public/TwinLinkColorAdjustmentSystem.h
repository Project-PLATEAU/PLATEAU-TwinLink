// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "Engine/DataTable.h"
#include "TwinLinkSystemVersionInfo.h"
#include "TwinLinkColorAdjustmentSystem.generated.h"

/**
 * 継承元マテリアル
 */
UENUM()
enum class ETwinLinkColorAdjustmentMaterialType : uint8 {
    Opaque,     //不透過
    Transparent, //透過
    Undefined,
};

/**
 * マテリアル設定情報
 */
USTRUCT()
struct FTwinLinkAdjustmentMaterialParameter : public FTableRowBase {
    GENERATED_BODY()
public:
    /** マテリアル表示名 **/
    UPROPERTY(BlueprintReadWrite)
        FString Name;
    /** マテリアル参照インデックス **/
    UPROPERTY(BlueprintReadWrite)
        int MaterialIndex;
    /** マテリアルタイプ **/
    UPROPERTY(BlueprintReadWrite)
        ETwinLinkColorAdjustmentMaterialType MaterialType;
    /** 拡散反射 **/
    UPROPERTY(BlueprintReadWrite)
        FLinearColor BaseColor;
    /** 鏡面反射 **/
    UPROPERTY(BlueprintReadWrite)
        FLinearColor SpecularColor;
    /** 放射**/
    UPROPERTY(BlueprintReadWrite)
        FLinearColor EmissiveColor;
    /** 光沢 **/
    UPROPERTY(BlueprintReadWrite)
        float Shininess;
    /** 透過度 **/
    UPROPERTY(BlueprintReadWrite)
        float Transparency;
    /**
     * @brief デフォルトコンストラクタ
    */
    FTwinLinkAdjustmentMaterialParameter() {
        Name = FString();
        MaterialIndex = 0;
        MaterialType = ETwinLinkColorAdjustmentMaterialType::Undefined;
        BaseColor = FLinearColor();
        SpecularColor = FLinearColor();
        EmissiveColor = FLinearColor();
        Shininess = 0.0f;
        Transparency = 0.0f;
    }
    /**
     * @brief コンストラクタ
    */
    FTwinLinkAdjustmentMaterialParameter(const FString& NewName, int NewMaterialIndex, TWeakObjectPtr<UMaterialInterface> NewMaterial) {
        Name = NewName;
        MaterialIndex = NewMaterialIndex;
        MaterialType = NewMaterial->GetBaseMaterial()->GetName().Equals(TEXT("PLATEAUX3DMaterial")) ? ETwinLinkColorAdjustmentMaterialType::Opaque : ETwinLinkColorAdjustmentMaterialType::Transparent;

        FLinearColor VectorValue;
        if (NewMaterial->GetVectorParameterValue(FName(TEXT("BaseColor")), VectorValue)) {
            BaseColor = FLinearColor(VectorValue.R, VectorValue.G, VectorValue.B);
        }
        if (NewMaterial->GetVectorParameterValue(FName(TEXT("SpecularColor")), VectorValue)) {
            SpecularColor = FLinearColor(VectorValue.R, VectorValue.G, VectorValue.B);
        }
        if (NewMaterial->GetVectorParameterValue(FName(TEXT("EmissiveColor")), VectorValue)) {
            EmissiveColor = FLinearColor(VectorValue.R, VectorValue.G, VectorValue.B);
        }

        float ScalarValue;
        if (NewMaterial->GetScalarParameterValue(FName(TEXT("Shininess")), ScalarValue)) {
            Shininess = ScalarValue;
        }
        if (NewMaterial->GetScalarParameterValue(FName(TEXT("Transparency")), ScalarValue)) {
            Transparency = ScalarValue;
        }
    }
};

/**
 * マテリアル出力情報
 */
USTRUCT()
struct FTwinLinkAdjustmentMaterialExport : public FTableRowBase {
    GENERATED_BODY()
public:
    /** アクタ名 **/
    FString ActorName;
    /** コンポーネント名 **/
    FString ComponentName;
    /** マテリアルスロット **/
    FString MaterialIndex;
    /** マテリアル参照インデックス **/
    int MaterialParamIndex;
public:
    /**
     * @brief デフォルトコンストラクタ
    */
    FTwinLinkAdjustmentMaterialExport() { ; }
    /**
     * @brief コンストラクタ
    */
    FTwinLinkAdjustmentMaterialExport(const FString& NewActorName, const FString& NewComponentName, const FString& NewMaterialIndex, const int NewMaterialParamIndex) {
        ActorName = NewActorName;
        ComponentName = NewComponentName;
        MaterialIndex = NewMaterialIndex;
        MaterialParamIndex = NewMaterialParamIndex;
    }
};

/**
 * 色彩調整機能のサブシステム
 */
UCLASS()
class TWINLINK_API UTwinLinkColorAdjustmentSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()
private:
    /** 色彩調整機能のバージョン　永続化時に組み込む **/
    const TwinLinkSystemVersionInfo VersionInfo = TwinLinkSystemVersionInfo(0, 0, 0);
    /** 色彩調整機能の永続化時のパス **/
    FString Filepath;
public:
    /** 設定されているマテリアル **/
    UPROPERTY()
        TArray<TWeakObjectPtr<UMaterialInterface>> Materials;
    /** マテリアルパラメータ **/
    UPROPERTY()
        TArray<FTwinLinkAdjustmentMaterialParameter> MaterialParameter;
    /** マテリアルパラメータ（オリジナル） **/
    UPROPERTY()
        TArray<FTwinLinkAdjustmentMaterialParameter> MaterialParameterOrigin;
    /** マテリアル出力データ **/
    UPROPERTY()
        TArray<FTwinLinkAdjustmentMaterialExport> MaterialExportData;
    float EmissibeStrength = 1000000;
public:
    /**
     * @brief モデルからマテリアル情報を抽出する
    */
    void ExtractMaterial();
    /**
     * @brief マテリアル情報をエクスポートする
    */
    void ExportColorAdjustmentParam();
    /**
     * @brief マテリアル情報をインポートする
    */
    void ImportColorAdjustmentParam();
    /**
     * @brief インポートデータのパース処理
    */
    bool Parse(const TArray<FString>& StrInfo);
};
