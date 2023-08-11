// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * データの永続化時のパスを作成するクラス
 * 
 */
class TWINLINK_API TwinLinkPersistentPaths
{
public:
    TwinLinkPersistentPaths() = delete;

    /**
     * 視点情報ファイルへのパスを作成する.
     * 
     * \param FileName　拡張子付きのファイル名
     * \return 
     */
    static FString CreateViewPointFilePath(const FString FileName);

    /**
     * アセット配置情報ファイルへのパスを作成する.
     *
     * \param FileName　拡張子付きのファイル名
     * \return
     */
    static FString CreateAssetPlacementFilePath(const FString FileName);

private:
    static FString GetProjSaveDir();
};
