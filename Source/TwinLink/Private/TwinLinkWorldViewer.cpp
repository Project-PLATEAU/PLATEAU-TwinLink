// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWorldViewer.h"

#include "PLATEAUCityObjectGroup.h"
#include "Kismet/GameplayStatics.h"
#include "TwinLinkCommon.h"
#include "TwinLinkMathEx.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"

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

    TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = GetComponentByClass<UCapsuleComponent>();
    _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Called every frame
void ATwinLinkWorldViewer::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // このクラスのプロパティの値で他のシステムの値を上書きする
    if (bOverrideOtherSystemValues) {
        CharMovementComponent->MaxFlySpeed = MaxFlySpeed;
        CharMovementComponent->MaxAcceleration = MaxAcceleration;
    }

    if (TargetTransform) {
        FVector NextLocation;
        FRotator NextRotation;
        if (TargetTransform->Update(DeltaTime, NextLocation, NextRotation))
            TargetTransform = std::nullopt;
        SetLocationImpl(NextLocation, NextRotation);
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

FVector ATwinLinkWorldViewer::GetNowCameraLocationOrZero() const {
    if (!GetController())
        return FVector::Zero();
    if (const auto ViewTarget = GetController()->GetViewTarget())
        return ViewTarget->GetActorLocation();
    return FVector::Zero();
}

FRotator ATwinLinkWorldViewer::GetNowCameraRotationOrDefault() const {
    if (!GetController())
        return FRotator();
    return GetControlRotation();

    //if (const auto ViewTarget = GetController()->GetViewTarget()) {
    //    ViewTarget->GetRot
    //    return ViewTarget->GetActorRotation();
    //}
    //return FRotator();
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
    const auto IsHit = GetLocalViewingPlayerController()->GetHitResultUnderCursorByChannel(
        UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

    // 任意のオブジェクトをクリックしたときのイベントを発行
    if (IsHit) {
        EvOnAnyObjectClicked.Broadcast(HitResult);
    }

    if (!HitResult.Component.IsValid()) {
        // 選択されていた地物がキャンセルされた時に通知する
        if (bIsSelectingFacility) {
            if (EvOnCanceledClickFacility.IsBound()) {
                EvOnCanceledClickFacility.Broadcast();
            }
            UE_TWINLINK_LOG(LogTemp, Log, TEXT("Canceled click facility"));
            bIsSelectingFacility = false;

            return;
        }
        return;
    }

    // bldg_:建物, tran_:道路, urf_:都市設備

    const TArray<FString> Filter{
        TEXT("bldg_")
    };

    // 想定されるプレフィックスが付与されているかチェックする
    auto bContainPrefix = false;

    constexpr auto MaxPrefixCnt = 6;    // テキトウな値 利用されるプレフィックスの最大文字数+_
    if (const auto ComponentName = HitResult.Component->GetName(); ComponentName.Len() >= MaxPrefixCnt) {
        const auto ProbablyPrefix = ComponentName.Left(MaxPrefixCnt);
        for (const auto F : Filter) {
            bContainPrefix = ProbablyPrefix.Contains(F, ESearchCase::CaseSensitive);
            if (bContainPrefix)
                break;
        }
    }

    if (!bContainPrefix) {
        // 名前で該当しない場合属性情報からチェックする（地物によっては処理重い場合あり）
        const auto CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(HitResult.Component);
        if (CityObjectGroup == nullptr)
            return;

        const auto& CityObjects = CityObjectGroup->GetAllRootCityObjects();
        if (CityObjects.IsEmpty())
            return;

        if (CityObjects[0].Type != EPLATEAUCityObjectsType::COT_Building)
            return;
    }

    // イベントを通知する
    if (EvOnClickedFacility.IsBound()) {
        EvOnClickedFacility.Broadcast(HitResult);
        bIsSelectingFacility = true;
    }
}

bool ATwinLinkWorldViewer::MoveInfo::Update(float DeltaSec, FVector& OutLocation, FRotator& OutRotation) {
    PassSec = FMath::Max(0.f, PassSec) + DeltaSec;
    if (PassSec >= MoveSec || MoveSec <= 0.f) {
        OutLocation = To.Location;
        OutRotation = To.Rotation;
        return true;
    }

    auto T = FMath::InterpEaseInOut(0.f, 1.f, PassSec / MoveSec, 1.f);

    OutLocation = FMath::Lerp(From.Location, To.Location, T);
    OutRotation = FMath::Lerp(From.Rotation, To.Rotation, T);
    return false;
}

void ATwinLinkWorldViewer::SetLocationImpl(const FVector& Position, const FRotator& Rotation) {
    GetController()->ClientSetLocation(Position, Rotation);
}

void ATwinLinkWorldViewer::SetLocation(const FVector& Position, const FRotator& Rotation, float MoveSec) {
    if (MoveSec <= 0.f) {
        SetLocationImpl(Position, Rotation);
        TargetTransform = std::nullopt;
    }
    else {
        TargetTransform = MoveInfo();
        TargetTransform->From = Transform{ GetNowCameraLocationOrZero(), GetNowCameraRotationOrDefault() };
        TargetTransform->To = Transform{ Position, Rotation };
        TargetTransform->MoveSec = MoveSec;
    }
}

void ATwinLinkWorldViewer::SetLocation(const FVector& Position, const FVector& RotationEuler, float MoveSec) {
    SetLocation(Position, FRotator::MakeFromEuler(RotationEuler), MoveSec);
}

void ATwinLinkWorldViewer::SetLocationLookAt(const FVector& Position, const FVector& LookAt, float MoveSec) {
    const auto Rotation = TwinLinkMathEx::CreateLookAtMatrix(Position, LookAt);
    SetLocation(Position, FRotator(Rotation.ToQuat()), MoveSec);
}
