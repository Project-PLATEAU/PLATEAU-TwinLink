// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSystemVersionInfo.h"
#include "ITwinLinkCSVAnalysis.h"

/**
 * CSVファイルのフォーマットの定義、解析機能を持つクラスの親クラス
 * 名前空間代わりに記述
 */
class TWINLINK_API TwinLinkCSVContents {
public:
    TwinLinkCSVContents() = default;
    ~TwinLinkCSVContents() = default;

public:
    /**
     * CSVファイルのフォーマットの定義、解析機能を持つクラス.
     */
    class Standard : public ITwinLinkCSVAnalysis {
    public:
        Standard(const TwinLinkSystemVersionInfo& Version);

        /**
         * TwinLinkのCSVファイルの解析.
         *
         * \param RowElements CSVのデータファイルを行単位に分割したもの
         */
        virtual bool Parse(TArray<FString> RowElements) override;

        /**
         * TwinLinkのCSVファイルの解析.
         *
         * \param RowElements CSVのデータファイルを行単位に分割したもの
         */
        virtual bool ParseOmit(TArray<FString> RowElements) override;

        /**
         * ヘッダーコンテンツの作成.
         *
         * 本来は作成したコンテンツをこのクラスのメンバーに設定した方が自然だが以下の理由で許容する
         * 　文字列→各型での値→文字列という変換が必要になる
         * 　一応、メンバーを変更しないconstが付いてる
         *
         * \param Version
         * \param KeyContents エスケープシーケンスを含まない文字列　　ex "名前, 年齢, 身長"
         * \return
         */
        FString CreateHeaderContents(FString HeaderContents) const;

        /**
         * ボディーコンテンツの作成.
         *
         * 本来は作成したコンテンツをこのクラスのメンバーに設定した方が自然だが以下の理由で許容する
         * 　文字列→各型での値→文字列という変換が必要になる
         * 　一応、メンバーを変更しないconstが付いてる
         *
         * \param MainContents エスケープシーケンスを含まない文字列　　ex "ほげほげ, 25, 170.0"
         * \return
         */
        FString CreateBodyContents(FString BodyContents) const;

        /**
         * バージョンの取得.
         *
         * \return
         */
        const TwinLinkSystemVersionInfo& GetVersion() { return Version; }

        /**
         * キーコンテンツの取得.
         *
         * \return
         */
        const TArray<FString>& GetKeyContents() { return KeyContents; }

        /**
         * ボディコンテンツの取得.
         *
         * \return
         */
        const TArray<TArray<FString>>& GetBodyContents() { return MainContents; }

    private:
        const TwinLinkSystemVersionInfo Version;
        TArray<FString> KeyContents;
        TArray<TArray<FString>> MainContents;
    };
};
