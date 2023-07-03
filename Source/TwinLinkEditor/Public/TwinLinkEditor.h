// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

class TWINLINKEDITOR_API ITwinLinkEditorModule : public IModuleInterface {
public:
    static ITwinLinkEditorModule& Get();
    static bool IsAvailable();

    virtual TSharedRef<class FTwinLinkWindow> GetWindow() = 0;
};
