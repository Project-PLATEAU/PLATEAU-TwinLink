// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWorldViewer.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ATwinLinkWorldViewer::ATwinLinkWorldViewer() {
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    CharMovementComponent = nullptr;
}

// Called when the game starts or when spawned
void ATwinLinkWorldViewer::BeginPlay() {
    Super::BeginPlay();

}

// Called every frame
void ATwinLinkWorldViewer::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    
    // このクラスのプロパティの値で他のシステムの値を上書きする
    if (bOverrideOtherSystemValues) {
        CharMovementComponent->MaxFlySpeed = MaxFlySpeed;
        CharMovementComponent->MaxAcceleration = MaxAcceleration;
    }

}

// Called to bind functionality to input
void ATwinLinkWorldViewer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    // InputMappingContext追加、入力アクションに対しての関数のバインド処理はBluePrint側で行っている。
    // 下記のURLに倣って記述したがバインド出来なかったため
    // https://docs.unrealengine.com/5.2/ja/enhanced-input-in-unreal-engine/

    // 空を飛べるようにする
    ClientCheatFly();

    // 移動コンポーネントを利用する前に取得
    CharMovementComponent = GetCharacterMovement();
    check(CharMovementComponent);
}

// カメラの前後移動
void ATwinLinkWorldViewer::MoveForward(const float Value) {
    if (Controller && Value != 0.0f) {
        const auto Rotation = Controller->GetControlRotation();
        const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
        AddMovementInput(Direction, Value * CameraMovementSpeed * GetWorld()->GetDeltaSeconds());
    }
}

// カメラの横移動
void ATwinLinkWorldViewer::MoveRight(const float Value) {
    if (Controller && Value != 0.0f) {
        const auto Rotation = Controller->GetControlRotation();
        const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
        AddMovementInput(Direction, Value * CameraMovementSpeed * GetWorld()->GetDeltaSeconds());
    }
}

// カメラの上下移動
void ATwinLinkWorldViewer::MoveUp(const float Value) {
    if (Controller && Value != 0.0f) {
        const auto Direction = FVector::UpVector;
        AddMovementInput(Direction, Value * CameraMovementSpeed * GetWorld()->GetDeltaSeconds());
    }
}

// カメラの水平回転
void ATwinLinkWorldViewer::Turn(const float Value) {
    AddControllerYawInput(Value * CameraRotationSpeed);
}

// カメラの垂直回転
void ATwinLinkWorldViewer::LookUp(const float Value) {
    AddControllerPitchInput(Value * CameraRotationSpeed);
}
