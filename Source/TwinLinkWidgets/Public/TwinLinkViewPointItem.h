// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkScrollBoxElementImpl.h"
#include "TwinLinkViewPointItem.generated.h"


/**
 * 視点情報を扱うScrollboxの要素のウィジェットの基底クラス
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkViewPointItem : public UTwinLinkScrollBoxElementImpl {
    GENERATED_BODY()

public:
    /**
     * @brief この要素のセットアップを行う
     * @param Data
    */
    virtual void OnSetup(UObject* Data) const override;

    /**
     * @brief 要素削除の要求を行う
     * @param Data
    */
    virtual void OnRequestRemove(UObject* Data) const override;

    /**
     * @brief 要素編集の要求を行う
    */
    virtual void OnRequestEdit(UObject* Data, const FString& NewName) const override;

    /**
     * @brief 要素の名前を取得する
     * @param Data
     * @return
    */
    virtual FText OnGetName(UObject* Data) const override;

    /**
     * @brief この要素を選択する
    */
    virtual void OnRequestSelect(UObject* Data) const override;

};
