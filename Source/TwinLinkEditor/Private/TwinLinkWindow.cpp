// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkWindow.h"

#include "EditorUtilityWidgetBlueprint.h"
#include "LevelEditor.h"
#include "Framework/Docking/LayoutExtender.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPLATEUTwinLinkEditorModule"

constexpr TCHAR WidgetPath[] = TEXT("/PLATEAU-TwinLink/EUW/MainWindow");

const FName FTwinLinkWindow::TabID(TEXT("TwinLinkWindow"));

void FTwinLinkWindow::Startup() {
    // メニューバーの拡張情報の作成
    // メイン画面を開くボタンを追加する。
    Extender = MakeShared<FExtender>();
    if (Extender.IsValid()) {
        Extender->AddMenuBarExtension(
            "Help",
            EExtensionHook::After,
            nullptr,
            FMenuBarExtensionDelegate::CreateRaw(this, &FTwinLinkWindow::CreateOpenWindowMenu)
        );
    }

    // メニューバーの拡張情報の登録
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
    const auto MenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
    if (MenuExtensibilityManager.IsValid()) {
        MenuExtensibilityManager->AddExtender(Extender);
    }
}

void FTwinLinkWindow::Shutdown() {
    // タブスポナーから登録解除
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabID);
}

TSharedRef<SDockTab> FTwinLinkWindow::SpawnTab(const FSpawnTabArgs& TabSpawnArgs) {
    EditorUtilityWidgetBlueprint = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, WidgetPath);
    if (EditorUtilityWidgetBlueprint != nullptr) {
        return EditorUtilityWidgetBlueprint->SpawnEditorUITab(TabSpawnArgs);
    }

    const FText Title = LOCTEXT("WidgetWarning", "ウィジェット破損");
    const FText DialogText = LOCTEXT("WidgetWarningDetail", "PLATEAU SDKのウィジェットが破損しています。");
    FMessageDialog::Open(EAppMsgType::Ok, DialogText, &Title);
    return SNew(SDockTab).ShouldAutosize(false).TabRole(NomadTab);
}

UEditorUtilityWidget* FTwinLinkWindow::GetEditorUtilityWidget() const {
    return EditorUtilityWidgetBlueprint->GetCreatedWidget();
}

bool FTwinLinkWindow::CanSpawnTab(const FSpawnTabArgs& TabSpawnArgs) const {
    if (!EditorUtilityWidgetBlueprint.IsValid()) {
        return true;
    }
    return EditorUtilityWidgetBlueprint.IsValid() && EditorUtilityWidgetBlueprint->GetCreatedWidget() == nullptr;
}

void FTwinLinkWindow::ConstructTab() {
    const auto GTabManager = FGlobalTabmanager::Get();
    GTabManager->RegisterNomadTabSpawner(TabID, FOnSpawnTab::CreateRaw(this, &FTwinLinkWindow::SpawnTab), FCanSpawnTab::CreateRaw(this, &FTwinLinkWindow::CanSpawnTab))
        .SetDisplayName(FText::FromString(TEXT("PLATEAU TwinLink")));

    const auto LevelEditorModule =
        FModuleManager::GetModulePtr<FLevelEditorModule>(
            FName(TEXT("LevelEditor")));
    if (LevelEditorModule != nullptr) {
        LevelEditorModule->OnRegisterLayoutExtensions().AddLambda(
            [](FLayoutExtender& InExtender) {
                // アセット配置タブと同じ位置(画面左側にドッキングされる)にタブを設定
                InExtender.ExtendLayout(
                    FTabId("PlacementBrowser"),
                    ELayoutExtensionPosition::After,
                    FTabManager::FTab(TabID, ETabState::OpenedTab));
            });
    }

    const auto TabManager =
        LevelEditorModule
        ? LevelEditorModule->GetLevelEditorTabManager()
        : FGlobalTabmanager::Get();
    TabManager->TryInvokeTab(TabID);
}

void FTwinLinkWindow::CreateOpenWindowMenu(FMenuBarBuilder& MenuBarBuilder) {
    MenuBarBuilder.AddMenuEntry(
        LOCTEXT("MenuTitle", "PLATEAU TwinLink"),
        LOCTEXT("PulldownMenuToolTip", "PLATEAU TwinLink画面を開く."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FTwinLinkWindow::ConstructTab)));
}

#undef LEVEL_EDITOR_NAME
#undef LOCTEXT_NAMESPACE
