// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

class UEditorUtilityWidgetBlueprint;
class UEditorUtilityWidget;

/**
 * @brief PLATEAU TwinLinkのメイン画面を表します。
 */
class TWINLINKEDITOR_API FTwinLinkWindow {
public:
    /**
     * @brief TwinLink画面を初期化します。
     */
    void Startup();

    /**
     * @brief TwinLink画面をシャットダウンします。
     */
    void Shutdown();

    /**
     * @brief スポーンされるEUWを取得します。
     * @return EUWのポインタ
     */
    UEditorUtilityWidget* GetEditorUtilityWidget() const;
private:
    TSharedPtr<FExtender> Extender;
    TWeakObjectPtr<UEditorUtilityWidgetBlueprint> EditorUtilityWidgetBlueprint;

    static const FName TabID;

    void CreateOpenWindowMenu(FMenuBarBuilder& MenuBarBuilder);
    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs);
    bool CanSpawnTab(const FSpawnTabArgs& TabSpawnArgs) const;
    void ConstructTab();
};
