// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/NativeWidgetHost.h"
#include "TwinLinkSelectActorComboBox.generated.h"

/**
 * アクタ選択コンボボックス
 */
UCLASS()
class TWINLINKEDITOR_API UTwinLinkSelectActorComboBox : public UNativeWidgetHost {
    GENERATED_BODY()
public:
    /**
     * @brief コンストラクタ
    */
    UTwinLinkSelectActorComboBox(const FObjectInitializer& ObjectInitialize);
public:
    /** 選択アクタ **/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Editor)
        TSoftObjectPtr<UObject> ModelActor;
};
