// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "TwinLinkFacilityImporter.generated.h"

class APLATEAUInstancedCityModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwinLinkOnLoadFacilityModel, const FString&, Message);

UCLASS()
class TWINLINKEDITOR_API UTwinLinkFacilityImporter : public UBlueprintAsyncActionBase {
public:
    GENERATED_BODY()

public:
    /** Execute the actual load */
    virtual void Activate() override;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "TwinLink", WorldContext = "WorldContextObject"))
    static UTwinLinkFacilityImporter* ImportAsync(UObject* WorldContextObject, APLATEAUInstancedCityModel* CityModel, const FString& GmlPath);

    // 非同期完了デリゲート
    UPROPERTY(BlueprintAssignable)
    FTwinLinkOnLoadFacilityModel Completed;

    // 非同期完了デリゲート
    UPROPERTY(BlueprintAssignable)
    FTwinLinkOnLoadFacilityModel Failed;

private:
    FString DoImport();

    UFUNCTION()
    void OnPostImport();

    TObjectPtr<APLATEAUInstancedCityModel> CityModel;
    TObjectPtr<class APLATEAUCityModelLoader> Loader;
    FString GmlPath;
};
