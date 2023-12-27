// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkWalkModePanelBase.generated.h"

UENUM(BlueprintType)
enum class ETwinLinkWalkModePanelBaseInputKey : uint8 {
    Q, E, W, S, A, D,
    _Undefind
};

/**
 * 
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkWalkModePanelBase : public UTwinLinkWidgetBase
{
	GENERATED_BODY()
	
public:
    /**
     * @brief コンストラクト
    */
    virtual void NativeConstruct() override;

    /**
     * @brief デストラクト
    */
    virtual void NativeDestruct() override;

    /**
     * @brief Tick
     * @param MyGeometry 
     * @param InDeltaTime 
    */
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup();


    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    bool IsActiveWalkMode() const;

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void StandbyClick();

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void CancelStandbyClick();

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void SetMovementSpeedMultiply(float Multiply);

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void DeactiveWalkMode();

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void BeginMoveViewer(ETwinLinkWalkModePanelBaseInputKey Key);

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void EndMoveViewer();

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void SetupWBP();

    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedWalkMode(bool bIsActive);

    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnFailedDeplayWalkModeViewer(const FString& Error);

private:
    bool BindClickEvnet();

private:
    bool bIsReadyWalkMode;

    FDelegateHandle EvOnAnyObjectClickedHnd;

    ETwinLinkWalkModePanelBaseInputKey CurrentMovement;

};
