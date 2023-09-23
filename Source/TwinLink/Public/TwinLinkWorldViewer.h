// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TwinLinkWorldViewer.generated.h"

// クリックイベントの取得
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FDelClickViewPort, FHitResult, HitResult);
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FDelCanceledClickFacility);

/**
 * @brief 視点操作機能を提供するカメラを搭載したキャラクタークラス
*/
UCLASS()
class TWINLINK_API ATwinLinkWorldViewer : public ACharacter {
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ATwinLinkWorldViewer();

    static TWeakObjectPtr<ATwinLinkWorldViewer> GetInstance(UWorld* World);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:


    /**
     * @brief 移動先の設定
     * 現在は指定した先に瞬間移動する
     * 後から高速移動に切り替えるかも
     * @param Position
     * @param Rotation
    */
    void SetLocation(const FVector& Position, const FRotator& Rotation, bool bForce = true);
    void SetLocation(const FVector& Position, const FVector& RotationEuler, bool bForce = true);


    /**
     * @brief 移動先の設定
     * 現在は指定した先に瞬間移動する
     * 後から高速移動に切り替えるかも
     * @param Position
     * @param LookAt
     * @param bForce : trueの時は即座に適用
    */
    void SetLocationLookAt(const FVector& Position, const FVector& LookAt, bool bForce = true);

private:
    void ATwinLinkWorldViewer::SetLocationImpl(const FVector& Position, const FRotator& Rotation);
public:
    // クリック
    FDelClickViewPort EvOnClickedFacility;
    FDelCanceledClickFacility EvOnCanceledClickFacility;

private:
    UFUNCTION(BlueprintCallable, Category = "Movement")
        FVector GetNowCameraLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
        FRotator GetNowCameraRotation() const;

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

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Click();

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

    bool bIsSelectingFacility;

    struct Transform
    {
        FVector Location;
        FRotator Rotation;
    };

    std::optional<Transform> TargetTransform;
};
