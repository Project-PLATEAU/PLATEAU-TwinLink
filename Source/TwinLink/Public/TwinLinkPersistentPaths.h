// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * データの永続化時のパスを作成するクラス
 *
 */
class TWINLINK_API TwinLinkPersistentPaths {
public:
    TwinLinkPersistentPaths() = delete;

    /**
     * @brief 視点情報ファイルへのパスを作成する
     * @param FileName 拡張子付きのファイル名
     * @return
    */
    static FString CreateViewPointFilePath(const FString& FileName);

    /**
     * @brief アセット配置情報ファイルへのパスを作成する.
     * @param FileName 拡張子付きのファイル名
     * @return
    */
    static FString CreateAssetPlacementFilePath(const FString& FileName);

    /**
     * @brief 施設情報ファイルへのパスを作成する
     * @param FileName 拡張子付きのファイル名
     * @return
    */
    static FString CreateFacilityInfoFilePath(const FString& FileName);

    /**
     * @brief 施設Imageへのパスを作成する
     * @param FileName 拡張子付きのファイル名
     * @return
    */
    static FString CreateFacilityImagePath(const FString& FileName);

    /**
     * @brief 建物内施設情報を保存するフォルダのパスを作成する
     * @param SubFolderName サブフォルダ名
     *  例 基本フォルダパス/bldg_AAA_1F
     * @return 
    */
    static FString CreateFloorInfoFolderPath();
    static FString CreateFloorInfoFolderPath(const FString& SubFolderName);

    /**
     * @brief 建物内施設情報を保存するファイルのパスを作成する
     * @param SubFolderName サブフォルダ名 (地物の階層単位で存在するフォルダ)
     * @param FileName 拡張子付きのファイル名
     * @return
    */
    static FString CreateFloorInfoFilePath(const FString& SubFolderName, const FString& FileName);

    /**
     * @brief 設計情報のイメージへのパスを作成する
     * @param FileName 
     * @return 
    */
    static FString CreateBuildingDesignImagePath(const FString& FileName);

    /**
     * @brief 設計情報ファイルへのパスを作成する
     * @param FileName 
     * @return 
    */
    static FString CreateBuildingDesignFilePath(const FString& FileName);

private:
    static FString GetProjSaveDir();
};
