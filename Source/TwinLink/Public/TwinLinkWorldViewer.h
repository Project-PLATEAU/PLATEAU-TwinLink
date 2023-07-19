// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TwinLinkWorldViewer.generated.h"

UCLASS()
class TWINLINK_API ATwinLinkWorldViewer : public ACharacter {
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ATwinLinkWorldViewer();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    // BluePrint側から呼び出す処理
    UFUNCTION(BlueprintCallable, Category = "Movement")
        void MoveForward(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void MoveRight(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void MoveUp(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void Turn(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void LookUp(const float Value);

private:
    /* カメラの移動速度 */
    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement")
        float CameraMovementSpeed = 500000.0f;

    /* カメラの回転速度 */
    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement")
        float CameraRotationSpeed = 1.0f;

    /* 最大加速度(速度の変化率) */
    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement", meta = (ClampMin = "0", UIMin = "0", ForceUnits = "%"))
        float MaxAcceleration = 20000.f;

    /* cm/sec 最大飛行速度 */
    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement", meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
        float MaxFlySpeed = 50000.0f;


    /* このプロパティ値で他のシステムの値を上書きする */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "TwinLink View Movement")
        bool bOverrideOtherSystemValues = true;


    UCharacterMovementComponent* CharMovementComponent;

};
