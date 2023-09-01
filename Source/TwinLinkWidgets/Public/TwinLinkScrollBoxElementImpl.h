// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TwinLinkScrollBoxElementImpl.generated.h"

/**
 * ScrollBoxの要素の実装クラス
 * これを継承して実装を拡張する
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkScrollBoxElementImpl : public UObject {
    GENERATED_BODY()

public:
    /**
     * @brief この要素のセットアップを行う
     * @param Data
    */
    virtual void OnSetup(UObject* Data) const {};

    /**
     * @brief この要素の削除要求を行う
     * @param Data
    */
    virtual void OnRequestRemove(UObject* Data) const {};

    /**
     * @brief この要素の編集要求を行う
    */
    virtual void OnRequestEdit(UObject* Data, const FString& NewName) const {};

    /**
     * @brief 要素の名前を取得する
     * @param Data
     * @return
    */
    virtual FText OnGetName(UObject* Data) const { return FText(); };

    /**
     * @brief この要素を選択する要求を行う
     * @param Data
    */
    virtual void OnRequestSelect(UObject* Data) const {};

};
