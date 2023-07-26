// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkScrollBoxBase.h"
#include "TwinLinkViewPointListPanel.generated.h"

/**
 * 視点情報を扱うパネルウィジェットの基底クラス
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkViewPointListPanel : public UTwinLinkScrollBoxBase
{
	GENERATED_BODY()

protected:
    /**
     * @brief TwinLinkのシステムと連携を行う
     * @param OutCollection 
    */
    virtual void OnSetupOnTwinLink(TWeakObjectPtr<UTwinLinkObservableCollection>* OutCollection) const override;
	
};
