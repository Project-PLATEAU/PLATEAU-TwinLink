// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"

#include "TwinLinkSystemVersionInfo.h"
#include "TwinLinkFloorInfoCollection.h"
#include "TwinLinkBuildingDesignInfo.h"

#include "TwinLinkFloorInfoSystem.generated.h"

// 施設内階層情報
class UTwinLinkFloorInfo;

// データ更新時のイベント
DECLARE_EVENT(UTwinLinkFloorInfoSystem, FDelOnChangedBuildingDesign);

/**
 * 施設内階層情報の管理システム
 */
UCLASS()
class TWINLINK_API UTwinLinkFloorInfoSystem : public UTwinLinkSubSystemBase
{
	GENERATED_BODY()

public:
    /**
     * @brief 初期化
     * @param Collection
    */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /**
     * @brief 解放処理
    */
    virtual void Deinitialize() override;

public:
    /**
     * @brief フロアの選択を行う
     * 非推奨　設計の抜け穴として作成したため
     * @param FeatureID 
     * @param InGrpID 
     * @param Key 
    */
    void SelectedFloor(const FString& FeatureID, const FString& InGrpID, const FString& Key);

    /**
     * @brief 選択フロアのキーを取得する
     * 非推奨　設計の抜け穴として作成したため
     * @return 
    */
    FString GetKeyBySelectedFloor();

    /**
     * @brief 選択フロアのGrpIDを取得する
     * 非推奨　設計の抜け穴として作成したため
     * @return 
    */
    FString GetGrpIDBySelectedFloor();

    /**
     * @brief 追加可能な施設情報かチェックする
     * @param Name
     * @return
    */
    bool CheckAddableFloorInfo(
        const FString& InName,
        const FString& InCategory,
        const FVector& InLocation,
        const FString& InImageFileName,
        const FString& InGuideText,
        const FString& InOpningHoursText);

    /**
     * @brief 正常な施設情報かチェックする
     * @param Name
     * @return
    */
    bool CheckValidFloorInfo(
        const FString& InName,
        const FString& InCategory,
        const FVector& InLocation,
        const FString& InImageFileName,
        const FString& InGuideText,
        const FString& InOpningHoursText);

    /**
     * @brief 施設情報を追加する
     * @param _Name
    */
    void AddFloorInfo(
        const FString& Key,
        const FString& InName,
        const FString& InCategory,
        const FVector& InLocation,
        const FString& InImageFileName,
        const FString& InGuideText,
        const FString& InOpningHoursText);

    /**
     * @brief 施設情報を削除する
     * @param FloorInfo
    */
    void RemoveFloorInfo(const FString& Key, const TWeakObjectPtr<UTwinLinkFloorInfo>& FloorInfo);

    /**
     * @brief 施設情報群をエキスポートする
    */
    void ExportFloorInfo();

    /**
     * @brief 施設情報群をインポートする
    */
    void ImportFloorInfo();

    /**
     * @brief 表示用カテゴリ文字列を変換する
     * @param Category 管理用カテゴリ
     * @return 表示用カテゴリ文字列(失敗時は空文字列)
    */
    FString ConvertSystemCategoryToDisplayCategoryName(FString Category);

    /**
     * @brief 管理用カテゴリを変換する
     * @param DisplayCategory 表示用カテゴリ文字列
     * @return 管理用カテゴリ(失敗時は空文字列)
    */
    FString ConvertDisplayCategoryNameToSystemCategory(FString DisplayCategory);

    /**
     * @brief 表示用カテゴリ文字列一覧を取得する
     * @return 
    */
    TArray<FString> GetCategoryDisplayNameCollection();

    /**
     * @brief 設計情報を検索
     * @param Key 
     * @return 
    */
    TWeakObjectPtr<UTwinLinkBuildingDesignInfo> FindBuildingDesign(const FString& Key);

    /**
     * @brief 設計情報の追加リクエスト
     * @param Key GetKeyBySelectedFloor()の値
     * @param ImageFileName 
     * @return 
    */
    void AddBuildingDesign(
        const FString& Key,
        const FString& ImageFileName);

    /**
     * @brief 設計情報の変更リクエスト
     * @param BuidlingDesignInfo 
     * @param ImageFileName 
     * @return 
    */
    bool EditBuildingDesign(
        const FString& Key, 
        const TWeakObjectPtr<UTwinLinkBuildingDesignInfo>& BuidlingDesignInfo, 
        const FString& ImageFileName);

    /**
     * @brief 設計情報の削除リクエスト
     * @param Key GetKeyBySelectedFloor()の値
     * @param BuidlingDesignInfoPtr データのポインターのポインター　Remove後にnullになる
     * @return 
    */
    bool RemoveBuildingDesign(const FString& Key, const TWeakObjectPtr<UTwinLinkBuildingDesignInfo> BuidlingDesignInfoPtr);

    /**
     * @brief 設計情報をエキスポートする
     * @return 
    */
    void ExportBuildingDesign();

    /**
     * @brief 設計情報をインポートする
     * @return 
    */
    void ImportBuildingDesign();


    /**
     * @brief 施設情報の編集
     * @param FloorInfo
     * @param Name
     * @param Category
     * @param ImageFileName
     * @param Guide
     * @param SpotInfo
     * @return 成否
    */
    bool EditFloorInfo(
        const TWeakObjectPtr<UTwinLinkFloorInfo>& FloorInfo,
        const FString& Name,
        const FString& Category,
        const FString& ImageFileName,
        const FString& Guide,
        const FString& SpotInfo);
	
    /**
     * @brief GetFloorInfoCollection()で渡すキーを生成する
     * @param FeatureID 地物ID
     * @param Floor 階層
     * @return キー
    */
    FString CreateFloorInfoMapKey(const FString& FeatureID, const FString& Floor);

    /**
     * @brief 施設情報群を保持する通知機能付きのコレクションを取得する
     * 存在しないキーが指定されて場合は空のコレクションをそのキーに追加する
     * @param Key CreateFloorInfoMapKey() で作成したキー
     * @return 
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> GetFloorInfoCollection(const FString& Key);

public:
    /** 設計情報が変更された時に呼び出される **/
    FDelOnChangedBuildingDesign EvOnAddedBuildingDesignInfoInstance;

private:
    /** 施設情報機能のバージョン　永続化時に組み込む **/
    const TwinLinkSystemVersionInfo VersionInfo = TwinLinkSystemVersionInfo(0, 0, 0);

    /** 設計情報のマップ フロア名キーに扱う **/
    UPROPERTY()
    TMap<FString, TObjectPtr<UTwinLinkBuildingDesignInfo>> BuidingDesingInfoMap;

    /** 設計情報永続化時のファイルパス **/
    FString BuildingDesingFilePath;

    /** 施設情報の永続化時のファイル名 **/
    FString FileName;

    /** 施設情報群 **/
    UPROPERTY()
    TMap<FString, TObjectPtr<UTwinLinkFloorInfoCollection>> FloorInfoCollectionMap;

    /** 建物内施設カテゴリをキーに表示用文字列を値に持つCollection **/
    TMap<FString, FString> DisplayCategoryMap;

    /** 表示用文字列をキーに建物内施設カテゴリを値に持つCollection **/
    TMap<FString, FString> CategoryMap;

    // ↓妥協してこの形で実装。

    /** 選択されている建物 **/
    FString SelectedFeatureID;
    /** 選択されているGrp **/
    FString SelectedGrpID;
    /** 選択されている階層情報 **/
    FString SelectedKey;
};
