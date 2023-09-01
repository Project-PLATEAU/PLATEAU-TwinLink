// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkScrollBoxElementImpl.h"
#include "TwinLinkFacilityItem.generated.h"

/**
 * 施設情報の要素Widgetの内部実装を行っているクラス
 * クラス名を変更する　内部実装を行っていることを示す名前に修正する
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkFacilityItem : public UTwinLinkScrollBoxElementImpl {
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
