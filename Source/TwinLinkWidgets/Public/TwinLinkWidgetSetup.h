// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TwinLinkWidgetSetup.generated.h"


class UTwinLinkScrollBoxBase;
class UTwinLinkScrollBoxElementImpl;
class UTwinLink3DUMGCanvasBase;

/**
 * 基底ウィジェットの設定クラス
 * 設定を行う関数が配置されている
 *
 * 増えてきたら整備予定
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkWidgetSetup : public UUserWidget {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "TwinLink Widget Setup")
    static void SetupViewPointList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl);

    UFUNCTION(BlueprintCallable, Category = "TwinLink Widget Setup")
    static void SetupFacilityInfoList(UTwinLinkScrollBoxBase* Widget, UTwinLinkScrollBoxElementImpl* ElementImpl);

};
