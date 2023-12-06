// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TwinLinkAssetPlacementInfo.generated.h"

/**
 * アセット配置情報クラス
 */
UCLASS()
class TWINLINK_API UTwinLinkAssetPlacementInfo : public UObject {
    GENERATED_BODY()

public:
    /**
     * @brief 初期化
     * @param PresetID
     * @param Position
     * @param RotationEuler
     * @return 成否
    */
    bool Setup(const int PlacementPresetID, const FVector& PlacementPosition, const FVector& PlacementRotationEuler, const FVector& PlacementScale);

    /**
     * @brief 初期化
     * @param StrInfo 各要素の値を示す文字列を格納したコンテナ
     * @return 成否
    */
    bool Setup(const TArray<FString>& StrInfo);

    /**
     * @brief プリセットIDの取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        int GetPresetID() const { return PresetID; }

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

    /**
     * @brief 拡縮情報の取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        FVector GetScale() const { return Scale; }

private:
    /** プリセットID **/
    int PresetID;
    /** 位置情報 **/
    FVector Position;
    /** 回転情報 **/
    FVector RotationEuler;
    /** 拡縮情報 **/
    FVector Scale;
};
