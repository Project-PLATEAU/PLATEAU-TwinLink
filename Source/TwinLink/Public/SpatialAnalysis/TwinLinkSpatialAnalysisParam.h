// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSpatialAnalysisPresenter.h"
#include "Engine/DataAsset.h"
#include "TwinLinkSpatialAnalysisParam.generated.h"

/**
 *
 */
UCLASS()
class TWINLINK_API UTwinLinkSpatialAnalysisParam : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure)
        TSubclassOf<ATwinLinkSpatialAnalysisPresenter> GetPresenerBp() const
    {
        return PresenterBp;
    }

    UPROPERTY(EditAnywhere, meta = (Comment = "アクターのBP"), Category = "Base")
        TSubclassOf<ATwinLinkSpatialAnalysisPresenter> PresenterBp;
};
