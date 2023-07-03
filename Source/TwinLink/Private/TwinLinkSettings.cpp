// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "TwinLinkSettings.h"

void UTwinLinkSettings::SetEncryptedPin(const int Input) {
    EncryptedPin = Input;
}

int UTwinLinkSettings::GetEncryptedPin() const {
    return EncryptedPin;
}

FString UTwinLinkSettings::GetFilePath() {
    return FPaths::ProjectSavedDir() / "TwinLinkSettings.ini";
}
