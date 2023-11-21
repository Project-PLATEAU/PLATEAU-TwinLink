// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include <iostream>
#include <memory>
#include "libshape/datatype.h"
#include "TwinLinkPOISubSystem.generated.h"

/**
 * GISデータ表示サブシステム
 */
UCLASS()
class TWINLINK_API UTwinLinkPOISubSystem : public UTwinLinkSubSystemBase {
    GENERATED_BODY()
public:
    /**
    * @brief シェイプファイルをインポート
    */
    void ImportShapefile(const TArray<FString>& Files);
    /**
    * @brief 属性情報を登録
    */
    void RegisterAttributes(const FString& AttributeName, int AttributeIndex);
    /**
    * @brief POIに使用するマテリアルを生成
    */
    void AssignMaterials(const FString& AttributeName);
    /**
    * @brief カラーを割り当てる
    * @return 割り当てたカラー
    */
    FColor AssignColor();
    /**
    * @brief 空間IDへ連携するPOI
    * @param 空間IDのBoundingBox（高さ方向はみない）
    * @param POI情報　TMap<登録名, TMap<名称, 座標>>
    * @return 取得POI数
    */
    int TwinLinkPOIGetInsidePOI(const FBox& Box, TMap<FString, TMap<FString, FVector>>& Ret);
public:
    /** 属性選択用のリスト **/
    TArray<FString> AttributeList;
    /** 属性選択用のインデックスリスト **/
    TArray<int> AttributeListIndex;
    /** 登録済みPOIキーリスト **/
    TArray<FString> AttributeNames;
    /** 登録済みPOI情報 <Attribute, <Name, Vector>> **/
    TMap<FString, TMap<FString, FVector>> RegisteredPOIs;
    /** POIアクタ **/
    TMap<FString, TArray<TObjectPtr<AActor>>> POIActors;
    /** POIに使用するマテリアルインスタンス **/
    TMap<FString, TObjectPtr<UMaterialInstanceDynamic>> MaterialCollection;
    /** マテリアルへ適用するカラー **/
    TArray<FColor> OriginColors {
        FColor::FromHex("#1ABC9CFF"),  // ターコイズ
            FColor::FromHex("#3498DAFF"),  // ブルー
            FColor::FromHex("#9C58B7FF"),  // パープル
            FColor::FromHex("#E74C3CFF"),  // レッド
            FColor::FromHex("#E77F22FF")   // オレンジ
    };
    /** カラーリスト **/
    TArray<FColor> AssignColors;
    /** 表示・非表示状態を退避 **/
    TMap<FString, bool> EscapeVisibility;
private:
    /**
    * @brief std::string（Shift-JIS）からFString（UTF-16）への変換
    */
    FString ConvertSJISToFString(const std::string& SjisString);
private:
    /** シェイプファイルの情報 **/
    std::shared_ptr<shp::Layer> Layer;
};
