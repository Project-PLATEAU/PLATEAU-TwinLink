// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "TwinLinkNavSystemDef.h"
#include "Components/Widget.h"
#include "GameFramework/Actor.h"
#include "TwinLinkNavSystemPathLocator.generated.h"

class UTwinLinkFacilityInfo;
class UNavigationSystemV1;

UCLASS()
class TWINLINK_API ATwinLinkNavSystemPathLocator : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATwinLinkNavSystemPathLocator();
protected:
    virtual void BeginPlay() override;
public:
    virtual void Tick(float DeltaSeconds) override;

    // 現在の状態
    UFUNCTION(BlueprintCallable)
        TwinLinkNavSystemPathLocatorState GetNowState() const;

    UFUNCTION(BlueprintCallable)
        bool TryGetShowWarnText(FVector2D& OutScreenPos, TwinLinkNavSystemPathLocatorState& OutState);

    /*
     * @brief : 警告テキストを表示する座標を取得する
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TwinLink")
        FVector GetWarnTextWorldLocation() const;

    // 位置更新する
    bool UpdateLocation(const UNavigationSystemV1* NavSys, const FHitResult& HitResult);

    // 位置更新する
    virtual bool UpdateLocation(const UNavigationSystemV1* NavSys, const FVector& Location);

    // 選択状態にする
    void Select();

    // 非選択状態にする
    void UnSelect();

    // 最後の有効な位置を取得
    std::optional<FVector> GetLastValidLocation() const;

    virtual ECollisionChannel GetCollisionChannel() const;
    virtual void BeginDestroy() override;
    virtual void Destroyed() override;

protected:
    // 現在の状態
    UPROPERTY(EditAnywhere, Category = TwinLink_Base)
        TwinLinkNavSystemPathLocatorState State = TwinLinkNavSystemPathLocatorState::Undefined;

    // 現在の状態
    UPROPERTY(EditAnywhere, Category = TwinLink_Base)
        bool IsSelected = false;

    // ピンが置けない場所にあるときに表示する警告ウィジット
    UPROPERTY(VisibleAnywhere, Category = TwinLink_Base)
        UUserWidget* WarnTextWidget = nullptr;

    // ピンが置けない場所にあるときに表示する警告ウィジットテンプレート
    UPROPERTY(EditAnywhere, Category = TwinLink_Editor)
        TSubclassOf<UUserWidget> WarnTextWidgetBp = nullptr;

    // 最後にValid状態になった位置
    std::optional<FVector> LastValidLocation = std::nullopt;

    // デバッグで変更する夜係数(負数だと無視)
    UPROPERTY(EditAnywhere, Category = TwinLink_Debug)
        float DebugNightCoef = -1.f;

};
