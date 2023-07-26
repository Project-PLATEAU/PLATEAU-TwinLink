// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Version情報
 * 
 * セマンティック バージョニング 2.0.0
 * https://semver.org/lang/ja/ 
 */
class TWINLINK_API TwinLinkSystemVersionInfo
{
public:
	TwinLinkSystemVersionInfo(int Major, int Minor, int Patch);
	TwinLinkSystemVersionInfo(FString Major, FString Minor, FString Patch);
	~TwinLinkSystemVersionInfo();

    /**
     * 互換性があるか.
     * Majorの比較
     * 
     * \param 比較対象のVersion
     * \return 
     */
    bool CheckCompatibility(const TwinLinkSystemVersionInfo& Version) const;

    /**
     * 同じ構造か.
     * Major, Minorの比較
     *
     * \param 比較対象のVersion
     * \return
     */
    bool CheckSameArchitecture(const TwinLinkSystemVersionInfo& Version) const;

    /**
     * 完全一致か.
     * Major, Minor, Patchの比較
     * 
     * \param 比較対象のVersion
     * \return 
     */
    bool CheckCompleteMatch(const TwinLinkSystemVersionInfo& Version) const;

    /**
     * 文字列に変換.
     * 
     * \return 
     */
    FString ToString() const;

private:
    const int Major;
    const int Minor;
    const int Patch;
};
