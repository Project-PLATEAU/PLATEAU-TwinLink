// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkNavSystemPathLocator.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemEntranceLocator.generated.h"

class ATwinLinkNavSystemPathLocator;
class ATwinLinkNavSystem;

UCLASS()
class TWINLINK_API ATwinLinkNavSystemEntranceLocator : public ATwinLinkNavSystemPathLocator {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemEntranceLocator();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    /*
     * @brief : パス検索の準備ができている(目的地が正しく選択されている)
     */
    virtual bool IsValidLocation() const;

    /*
     * @brief : 指定したポイントの位置を取得
     */
    UFUNCTION(BlueprintCallable)
        bool TryGetLocation(FVector& Out) const;

    std::optional<FVector> GetPathLocation() const;

    /*
     * @brief : 指定したポイントの位置を設定.
     */
    UFUNCTION(BlueprintCallable)
        void SetEntranceLocation(UTwinLinkFacilityInfo* Info);

    /*
     * @brief : 指定したポイントの位置を設定.
     */
    UFUNCTION(BlueprintCallable)
        void SetDefaultEntranceLocation(const FString& FeatureId);

private:
    // 現在選択しているアクターのスクリーン位置オフセット
    UPROPERTY(EditAnywhere, Category = TwinLink_Path)
        FVector2D NowSelectedPathLocatorActorScreenOffset = FVector2D::Zero();
};