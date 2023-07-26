// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * CSVエキスポート機能を持ったクラス
 */
class TWINLINK_API TwinLinkCSVExporter
{
public:
	TwinLinkCSVExporter();
	~TwinLinkCSVExporter();

    /**
     * ヘッダーコンテンツの設定.
     * 
     * \param Header
     */
    void SetHeaderContents(const FString Header);
    void SetHeaderContents(const TArray<FString> Header);

    /**
     * ボディコンテンツの追加.
     * 
     * \param Contents 行を追加   ex "name, 1, 2"
     */
    void AddBodyContents(const FString Contents);

    /**
     * CSVファイルの出力.
     * 
     * \param Filepath 拡張子を含めたファイルパス
     * \return 成功時 true
     */
    bool ExportCSV(const TCHAR* Filepath);

private:
    FString HeaderContents;
    FString BodyContents;

};
