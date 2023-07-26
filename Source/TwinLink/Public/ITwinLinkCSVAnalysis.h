// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * CSV解析を行う際のインターフェイス
 */
class TWINLINK_API ITwinLinkCSVAnalysis
{
public:
    /**
     * TwinLinkのCSVファイルの解析.
     * 
     * \param RowElements CSVのデータファイルを行単位に分割したもの
     */
    virtual bool Parse(TArray<FString> RowElements) = 0;

};
