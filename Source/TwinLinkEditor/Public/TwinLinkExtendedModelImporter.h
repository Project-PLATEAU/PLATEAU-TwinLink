// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "TwinLinkExtendedModelImporter.generated.h"

class APLATEAUInstancedCityModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwinLinkOnLoadExtendedModel, const FString&, Message);

UCLASS()
class TWINLINKEDITOR_API UTwinLinkExtendedModelImporter : public UBlueprintAsyncActionBase {
public:
    GENERATED_BODY()

public:
    /** Execute the actual load */
    virtual void Activate() override;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "TwinLink", WorldContext = "WorldContextObject"))
    static UTwinLinkExtendedModelImporter* ImportAsync(UObject* WorldContextObject, APLATEAUInstancedCityModel* CityModel, const double ExtensionDistance);

    // 非同期完了デリゲート
    UPROPERTY(BlueprintAssignable)
    FTwinLinkOnLoadExtendedModel Completed;

    // 非同期完了デリゲート
    UPROPERTY(BlueprintAssignable)
    FTwinLinkOnLoadExtendedModel Failed;

private:
    FString DoImport();

    TObjectPtr<APLATEAUInstancedCityModel> CityModel;
    TObjectPtr<class APLATEAUCityModelLoader> Loader;
    double ExtensionDistance;
};
