// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkEditor.h"
#include "TwinLinkWindow.h"

ITwinLinkEditorModule& ITwinLinkEditorModule::Get() {
    return FModuleManager::LoadModuleChecked<ITwinLinkEditorModule>("TwinLinkEditor");
}

bool ITwinLinkEditorModule::IsAvailable() {
    return FModuleManager::Get().IsModuleLoaded("TwinLinkEditor");
}

class FTwinLinkEditorModule : public ITwinLinkEditorModule {
public:
    virtual void StartupModule() override {
        Window = MakeShareable(new FTwinLinkWindow());

        Window->Startup();
    }

    virtual void ShutdownModule() override {
        Window->Shutdown();
    }

    virtual TSharedRef<FTwinLinkWindow> GetWindow() override {
        return Window.ToSharedRef();
    }
    
private:
    TSharedPtr<FTwinLinkWindow> Window;
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTwinLinkEditorModule, TwinLinkEditor)
