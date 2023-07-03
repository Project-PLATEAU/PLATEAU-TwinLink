// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TwinLinkSettings.generated.h"

/**
 * @brief プラグイン設定データ
 */
UCLASS(config = Game)
class TWINLINK_API UTwinLinkSettings : public UObject {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void SetEncryptedPin(const int Input);

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        int GetEncryptedPin() const;

    static FString GetFilePath();

private:
    UPROPERTY(Config)
        int EncryptedPin = 0;

};
