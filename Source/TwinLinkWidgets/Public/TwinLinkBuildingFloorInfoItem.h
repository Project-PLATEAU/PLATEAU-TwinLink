// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkScrollBoxElementImpl.h"
#include "TwinLinkBuildingFloorInfoItem.generated.h"

/**
 * フロア情報のアイテムの実装クラス
 * リストのアイテムとして利用している
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkBuildingFloorInfoItem : public UTwinLinkScrollBoxElementImpl
{
	GENERATED_BODY()

protected:
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

private:
};
