// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TwinLinkAuthentication.generated.h"

UENUM(BlueprintType)
enum class ETwinLinkAuthenticationResult : uint8 {
    None,
    Success,
    InvalidLength,
    InvalidCharacters,
    Weak,
    InternalError,
    WrongPinCode
};

/**
 * @brief 数字4桁の暗証番号による認証機能を提供します。
 */
class TWINLINK_API FTwinLinkAuthentication {
public:
    FTwinLinkAuthentication() = delete;

    /**
    * @brief 暗証番号を設定します。
    * @return 設定が成功した場合はtrue、それ以外はfalse
    */
    static ETwinLinkAuthenticationResult SetPin(const FString& Input);

    /**
    * @brief パスワード認証を行います。
    * @return 認証に成功した場合はtrue、それ以外はfalse
    */
    static ETwinLinkAuthenticationResult Authenticate(const FString& Input);
};

UCLASS()
class TWINLINK_API UTwinLinkAuthenticationBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

        UFUNCTION(BlueprintCallable, Category = "TwinLink")
        static ETwinLinkAuthenticationResult SetAuthenticationPin(const FString& Input) {
        return FTwinLinkAuthentication::SetPin(Input);
    }

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        static ETwinLinkAuthenticationResult Authenticate(const FString& Input) {
        return FTwinLinkAuthentication::Authenticate(Input);
    }
};