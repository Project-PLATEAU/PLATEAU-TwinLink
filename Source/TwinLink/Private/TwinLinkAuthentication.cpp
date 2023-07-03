// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkAuthentication.h"

#include "TwinLinkSettings.h"

namespace {
    /**
     * @brief XOR暗号で簡易的に暗号化・復号します。
     * @param Input 暗号化・復号したいint値
     * @return 暗号化・復号されたint値
    */
    int XorEncryptDecrypt(const int Input) {
        constexpr int Key = 911315;

        return Input ^ Key;
    }

    /**
     * @brief 文字列をintに変換します。
     * @param InValue
     * @param OutValue
     * @return 変換が成功したかどうか
    */
    ETwinLinkAuthenticationResult ConvertToInt(const FString& InValue, int& OutValue) {
        if (InValue.Len() != 4)
            return ETwinLinkAuthenticationResult::InvalidLength;

        if (InValue == "0000") {
            OutValue = 0;
            return ETwinLinkAuthenticationResult::Weak;
        }

        OutValue = FCString::Atoi(*InValue);

        return OutValue != 0
            ? ETwinLinkAuthenticationResult::Success
            : ETwinLinkAuthenticationResult::InvalidCharacters;
    }
}

ETwinLinkAuthenticationResult FTwinLinkAuthentication::SetPin(const FString& Input) {
    int Pin;
    const auto Result = ConvertToInt(Input, Pin);
    if (Result != ETwinLinkAuthenticationResult::Success)
        return Result;

    // 暗号化
    const auto EncryptedPin = XorEncryptDecrypt(Pin);

    // 設定保存
    const auto Settings = GetMutableDefault<UTwinLinkSettings>();
    Settings->LoadConfig(nullptr, *UTwinLinkSettings::GetFilePath());
    Settings->SetEncryptedPin(EncryptedPin);
    Settings->SaveConfig(CPF_Config, *UTwinLinkSettings::GetFilePath());

    return ETwinLinkAuthenticationResult::Success;
}

ETwinLinkAuthenticationResult FTwinLinkAuthentication::Authenticate(const FString& Input) {
    int InputAsInt;
    const auto Result = ConvertToInt(Input, InputAsInt);
    if (Result != ETwinLinkAuthenticationResult::Success)
        return Result;

    // 設定読み込み
    const auto Settings = GetMutableDefault<UTwinLinkSettings>();
    Settings->LoadConfig(nullptr, *UTwinLinkSettings::GetFilePath());
    const auto EncryptedPin = Settings->GetEncryptedPin();

    // 復号
    const int Pin = XorEncryptDecrypt(EncryptedPin);
    return Pin == InputAsInt
        ? ETwinLinkAuthenticationResult::Success
        : ETwinLinkAuthenticationResult::WrongPinCode;
}
