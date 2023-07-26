// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TwinLinkViewPointInfo.generated.h"

/**
 * TwinLinkの視点情報クラス
 */
UCLASS(BlueprintType)
class TWINLINK_API UTwinLinkViewPointInfo : public UObject
{
	GENERATED_BODY()
	
public:   
    /**
     * @brief 初期化
     * @param Name 
     * @param Position 
     * @param RotationEuler 
     * @return 成否
    */
    bool Setup(FString ViewPointName, FVector ViewPointPosition, FVector ViewPointRotationEuler);

    /**
     * @brief 初期化
     * @param StrInfo 各要素の値を示す文字列を格納したコンテナ 
     * @return 成否
    */
    bool Setup(TArray<FString> StrInfo);

    /**
     * @brief 名前の取得
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        FString GetName() const { return Name; }

    /**
     * @brief 位置情報の取得
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        FVector GetPosition() const { return Position; }

    /**
     * @brief 回転情報の取得
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        FVector GetRotationEuler() const { return RotationEuler; }

private:
    FString Name;
    FVector Position;
    FVector RotationEuler;

};
