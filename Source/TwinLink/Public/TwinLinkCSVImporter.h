// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

// CSV解析クラスのインターフェイス
class ITwinLinkCSVAnalysis;

/**
 * CSVImport機能を提供するクラス
 */
class TWINLINK_API TwinLinkCSVImporter
{
public:
	TwinLinkCSVImporter();
	~TwinLinkCSVImporter();

    /**
     * CSVファイルのインポート.
     * 
     * \param Filepath 拡張子を含めたファイルパス
     * \param Analysis インポートしたファイルを解析するクラス
     * \return 成功時 true
     */
    bool ImportCSV(const TCHAR* Filepath, ITwinLinkCSVAnalysis* Analysis);

};
