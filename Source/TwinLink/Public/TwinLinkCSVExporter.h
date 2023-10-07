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
     * @brief ヘッダーコンテンツの設定.
     * 
     * @param Header
     */
    void SetHeaderContents(const FString Header);

    /**
     * @brief ヘッダーコンテンツの設定.
     * 注意　未テスト
     * @param Header 
    */
    void SetHeaderContents(const TArray<FString> Header);

    /**
     * @brief ボディコンテンツの追加.
     * 
     * @param Contents 行を追加   ex "name, 1, 2"
     */
    void AddBodyContents(const FString Contents);

    /**
     * @brief CSVファイルの出力.
     * 
     * @param Filepath 拡張子を含めたファイルパス
     * @return 成功時 true
     */
    bool ExportCSV(const TCHAR* Filepath);

    /**
     * @brief 内部のコンテンツをリセットする
     * このクラスを使いまわす際に利用する。
     * エキスポートして新規でコンテンツを追加する前に呼ぶ。
     * @return 
    */
    void ResetBodyContents();

    /**
     * @brief 内部のコンテンツを所持しているか
     * AddBodyContents()で空文字が追加された場合やそもそも呼んでいない場合は falseが返る
     * @return 
    */
    bool IsHasBodyContents();

private:
    FString HeaderContents;
    FString BodyContents;

};
