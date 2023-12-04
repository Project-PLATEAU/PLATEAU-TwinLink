// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkCSVContents.h"

TwinLinkCSVContents::Standard::Standard(const TwinLinkSystemVersionInfo& Version)
    : Version(Version) {
}

bool TwinLinkCSVContents::Standard::Parse(TArray<FString> RowElements) {
    KeyContents.Reset();
    MainContents.Reset();

    const auto bIsEmpty = RowElements.IsEmpty();
    ensure(bIsEmpty == false);
    if (bIsEmpty)
        return false;

    // Headerの情報を保存する
    TArray<FString> VersionStrings;
    RowElements[0].ParseIntoArray(VersionStrings, TEXT("."), true);

    // Versionチェック
    const auto bHasCompatibility = Version.CheckCompatibility(
        TwinLinkSystemVersionInfo(VersionStrings[0], VersionStrings[1], VersionStrings[2]));

    ensure(bHasCompatibility);
    if (bHasCompatibility == false) {
        return false;
    }

    RowElements[1].ParseIntoArray(KeyContents, TEXT(","), true);

    // Headerの情報を取り除く
    RowElements.RemoveAt(0, 2);
 
    // Bodyの情報を保存する
    TArray<FString> TempBuffer;
    for (const auto& Elements : RowElements) {
        // データの改行コードを復元する
        const auto Converted = Elements.Replace(TEXT("~n"), TEXT("\n"), ESearchCase::CaseSensitive);

        // 行を分解
        Converted.ParseIntoArray(TempBuffer, TEXT(","), false);

        // データのカンマを復元する
        for (FString& Temp : TempBuffer) {
            Temp = Temp.Replace(TEXT("~c"), TEXT(","), ESearchCase::CaseSensitive);
        }

        // データのチルダを復元する
        for (FString& Temp : TempBuffer) {
            Temp = Temp.Replace(TEXT("0TiLdE0"), TEXT("~"), ESearchCase::CaseSensitive);
        }

        // データ追加
        MainContents.Add(TempBuffer);
    }

    return true;
}

bool TwinLinkCSVContents::Standard::ParseOmit(TArray<FString> RowElements) {
    KeyContents.Reset();
    MainContents.Reset();

    const auto bIsEmpty = RowElements.IsEmpty();
    ensure(bIsEmpty == false);
    if (bIsEmpty)
        return false;

    // Headerの情報を保存する
    RowElements[0].ParseIntoArray(KeyContents, TEXT(","), true);

    // Headerの情報を取り除く
    RowElements.RemoveAt(0);

    // Bodyの情報を保存する
    TArray<FString> TempBuffer;
    for (const auto& Elements : RowElements) {
        Elements.ParseIntoArray(TempBuffer, TEXT(","), false);
        MainContents.Add(TempBuffer);
    }

    return true;
}
FString TwinLinkCSVContents::Standard::CreateHeaderContents(FString HeaderContents) const {
    return FString::Printf(TEXT("%s\n%s"), *(Version.ToString()), *HeaderContents);
}

FString TwinLinkCSVContents::Standard::CreateBodyContents(FString BodyContents) const {
    return FString::Printf(TEXT("%s"), *BodyContents);
}

FString TwinLinkCSVContents::Standard::CreateBodyContents(const TArray<FString>& Contents) const {
    FString Str;
    for (const auto Content : Contents) {
        FString Converted;
        Converted = Content.Replace(TEXT("~"), TEXT("0TiLdE0"), ESearchCase::CaseSensitive);
        Converted = Content.Replace(TEXT(","), TEXT("~c"), ESearchCase::CaseSensitive);

        Str.Append(FString::Printf(TEXT("%s,"), *Converted));
    }
    Str.RemoveAt(Str.Len() - 1);
    return Str;
}
