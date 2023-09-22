// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TwinLinkEditorNavSystemParam.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class TWINLINKEDITOR_API UTwinLinkEditorNavSystemParam : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    /*
     * @brief プライマリアセットID名を取得
     */
    UFUNCTION(BlueprintCallable)
        FString GetIdentifierString() const;


    /*
    * @brief プライマリアセットIDを取得
    */
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

    // ナビメッシュ生成時にエディタに作成されるランタイム用のNavSystemのBP
    UPROPERTY(EditAnywhere, Category = Editor)
        TSubclassOf<class ATwinLinkNavSystem> NavSystemBp;
};
