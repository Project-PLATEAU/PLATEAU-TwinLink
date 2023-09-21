// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWorldViewer.h"
#include "Kismet/GameplayStatics.h"
#include "TwinLinkCommon.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ATwinLinkWorldViewer::ATwinLinkWorldViewer() {
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    CharMovementComponent = nullptr;

    bIsSelectingFacility = false;
}

TWeakObjectPtr<ATwinLinkWorldViewer> ATwinLinkWorldViewer::GetInstance(UWorld* World) {
    AActor* WorldViewerActor = UGameplayStatics::GetActorOfClass(World, ATwinLinkWorldViewer::StaticClass());
    if (WorldViewerActor == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("failed get actor"));
        return nullptr;
    }

    ATwinLinkWorldViewer* WorldViewer = Cast<ATwinLinkWorldViewer>(WorldViewerActor);
    if (WorldViewer == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("failed get world viewer"));
        return nullptr;
    }

    return WorldViewer;
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

void ATwinLinkWorldViewer::Click() {
    FHitResult HitResult;
    if (!GetLocalViewingPlayerController())
        return;
    GetLocalViewingPlayerController()->GetHitResultUnderCursorByChannel(
        UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

    // bldg_:建物, tran_:道路, urf_:都市設備

    const TArray<FString> Filter{
        TEXT("bldg_")
    };

    if (HitResult.Component.IsValid()) {

        // 想定されるプレフィックスが付与されているかチェックする
        const auto MaxPrefixCnt = 6;    // テキトウな値 利用されるプレフィックスの最大文字数+_
        const auto ProbablyPrefix = HitResult.Component->GetName().Left(MaxPrefixCnt);
        auto isContains = false;
        for (const auto F : Filter) {
            isContains = ProbablyPrefix.Contains(F, ESearchCase::CaseSensitive);
            if (isContains)
                break;
        }

        // イベントを通知する
        if (isContains) {
            if (EvOnClickedFacility.IsBound()) {
                EvOnClickedFacility.Broadcast(HitResult);
                bIsSelectingFacility = true;
            }
        }
    }
    else {
        // 選択されていた地物がキャンセルされた時に通知する
        if (bIsSelectingFacility) {
            if (EvOnCanceledClickFacility.IsBound()) {
                EvOnCanceledClickFacility.Broadcast();
            }
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Canceled click facility"));
            bIsSelectingFacility = false;
        }
    }

}

void ATwinLinkWorldViewer::SetLocation(const FVector& Position, const FRotator& Rotation) {
    GetController()->ClientSetLocation(Position, Rotation);
}

void ATwinLinkWorldViewer::SetLocation(const FVector& Position, const FVector& RotationEulur) {
    GetController()->ClientSetLocation(Position, FRotator::MakeFromEuler(RotationEulur));
}
