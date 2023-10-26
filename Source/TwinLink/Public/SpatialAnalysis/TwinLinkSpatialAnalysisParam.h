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
        TSubclassOf<ATwinLinkSpatialAnalysisPresenter> GetPresenerBp() const {
        return PresenterBp;
    }

    UFUNCTION(BlueprintPure)
        int GetPeopleFlowRequestIntervalSeconds() const {
        return PeopleFlowRequestIntervalSeconds;
    }
    UFUNCTION(BlueprintPure)
        int GetDefaultZoom() const {
        return DefaultZoom;
    }

    UPROPERTY(EditAnywhere, meta = (Comment = "アクターのBP"), Category = "Base")
        TSubclassOf<ATwinLinkSpatialAnalysisPresenter> PresenterBp;

    UPROPERTY(EditAnywhere, meta = (Comment = "人流データ更新間隔"), Category = "Base")
        int PeopleFlowRequestIntervalSeconds = 60;

    UPROPERTY(EditAnywhere, meta = (Comment = "初期ズームレベル"), Category = "Base")
        int DefaultZoom = 18;
};
