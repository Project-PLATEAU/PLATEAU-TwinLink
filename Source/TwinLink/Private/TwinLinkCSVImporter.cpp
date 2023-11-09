// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkCSVImporter.h"
#include "ITwinLinkCSVAnalysis.h"

TwinLinkCSVImporter::TwinLinkCSVImporter() {
}

TwinLinkCSVImporter::~TwinLinkCSVImporter() {
}

bool TwinLinkCSVImporter::ImportCSV(const TCHAR* Filepath, ITwinLinkCSVAnalysis* Analysis) {
    //ファイル取得
    FString CsvFullData;
    const auto bIsSucLoad = FFileHelper::LoadFileToString(CsvFullData, Filepath);

    // ファイルの読み込みに失敗
    if (bIsSucLoad == false) {
        return false;
    }

    //列で分解
    TArray<FString> RowElements;
    CsvFullData.ParseIntoArray(RowElements, TEXT("\n"), true);

    // CSVデータの解析
    const auto bIsSucParse = Analysis->Parse(RowElements);

    // 解析に失敗
    if (bIsSucParse == false) {
        return false;
    }

    return true;
}

bool TwinLinkCSVImporter::ImportCSVOmit(const TCHAR* Filepath, ITwinLinkCSVAnalysis* Analysis) {
    //ファイル取得
    FString CsvFullData;
    const auto bIsSucLoad = FFileHelper::LoadFileToString(CsvFullData, Filepath);

    // ファイルの読み込みに失敗
    if (bIsSucLoad == false) {
        return false;
    }

    CsvFullData = CsvFullData.Replace(TEXT("\r\n"), TEXT("\n"));

    //列で分解
    TArray<FString> RowElements;
    CsvFullData.ParseIntoArray(RowElements, TEXT("\n"), true);

    // CSVデータの解析
    const auto bIsSucParse = Analysis->ParseOmit(RowElements);

    // 解析に失敗
    if (bIsSucParse == false) {
        return false;
    }

    return true;
}