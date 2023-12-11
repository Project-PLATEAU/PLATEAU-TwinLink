// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWorldViewer.h"

#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUCityObjectGroup.h"
#include "Kismet/GameplayStatics.h"
#include "TwinLinkCommon.h"
#include "Misc/TwinLinkMathEx.h"
#include "TwinLinkPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "NavSystem/TwinLinkNavSystem.h"

namespace {
    void RotateAroundFocusPoint(ATwinLinkWorldViewer* Self, const FVector& FocusPoint, const FRotator& BaseRotator, const FRotator& AddRotator, double OffsetDistance) {
        const auto LimitX = 85;
        const auto OffsetRotationNormalized = BaseRotator.GetNormalized();
        const auto RotatorX = FMath::Clamp(OffsetRotationNormalized.Pitch + AddRotator.Pitch, -LimitX, LimitX);
        const auto RotatorY = OffsetRotationNormalized.Yaw + AddRotator.Yaw;
        const auto RotatorZ = OffsetRotationNormalized.Roll + AddRotator.Roll;
        const auto Rotator = FRotator(RotatorX, RotatorY, RotatorZ);

        const auto NextForward = Rotator.Vector();
        const auto NextPos = FocusPoint - NextForward * OffsetDistance;

        Self->SetLocation(NextPos, Rotator);
    }

    TWeakObjectPtr<APLATEAUInstancedCityModel> CityModel;
    TWeakObjectPtr<UPLATEAUCityObjectGroup> DemCityModel;

}

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

    CurrentAutoViewMode = ETwinLinkViewMode::Manual;
}

// Called every frame
void ATwinLinkWorldViewer::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // 放置時間の計測
    bool bIsReqAutomode = false;
    if (NoInputProcessTime < LimitBeginAutoViewControlModeTime) {
        NoInputProcessTime += DeltaTime;
        // 放置時間が自動閲覧モード以降基準に達した
        if (NoInputProcessTime > LimitBeginAutoViewControlModeTime) {
            bIsReqAutomode = true;
        }
    }

    // 自動閲覧モードの有効化
    if (bIsReqAutomode) {
        ActivateAutoViewControlButton(ETwinLinkViewMode::FreeAutoView);
    }

    // 自動閲覧モードの更新処理　回転
    if (!CanReceivePlayerInput()) {
        NoInputProcessTime += DeltaTime;

        // 注視点回転
        switch (CurrentAutoViewMode) {
        case ETwinLinkViewMode::FreeAutoView:
        {
            AutoFreeViewControl.Update(this, DeltaTime);

        }

            break;
        case ETwinLinkViewMode::LimitedAutoView:
        {
            if (SingleFocusPoint.has_value()) {
                const auto Progress = AutoRotateViewProcessTime;
                RotateAroundFocusPoint(this, SingleFocusPoint.value(),
                    OffsetRotation, FRotator(0.0, AutoRotationSpeed * Progress, 0.0), OffsetLength);
            }
            else {
                AddControllerYawInput(AutoRotationSpeed * DeltaTime * CameraRotationSpeed);
            }
            AutoRotateViewProcessTime += DeltaTime;
        }
            break;
        }
    }

    // このクラスのプロパティの値で他のシステムの値を上書きする
    if (bOverrideOtherSystemValues) {
        CharMovementComponent->MaxFlySpeed = MaxFlySpeed;
        CharMovementComponent->MaxAcceleration = MaxAcceleration;
        CharMovementComponent->BrakingDecelerationFlying = BrakingDecelerationFlying;
    }

    if (TargetTransform) {
        FVector NextLocation;
        FRotator NextRotation;
        if (TargetTransform->Update(DeltaTime, NextLocation, NextRotation))
            TargetTransform = std::nullopt;

        SetLocationImpl(NextLocation, NextRotation);
    }

    // 遠くのものでもちゃんとヒットするように領域のAabbを貫くような長さをPlayerControllerに設定する
    if (const auto NavSystem = ATwinLinkNavSystem::GetInstance(GetWorld())) {
        const auto PlayerController = GetLocalViewingPlayerController();
        const auto MousePos = ATwinLinkPlayerController::GetMousePosition(PlayerController);
        if (MousePos.has_value()) {
            const auto Line = ATwinLinkPlayerController::ScreenToWorldRayThroughBoundingBox(PlayerController, *MousePos, NavSystem->GetFieldAabb());
            if (Line.has_value()) {
                PlayerController->HitResultTraceDistance = (Line->Max - Line->Min).Length();
            }
        }
    }

    // 更新されたかチェック
    const auto CurrentLocation = GetActorLocation();
    const auto CurrentRotation = GetActorRotation();
    if (PreLocation != CurrentLocation || PreRotation != CurrentRotation) {
        // 更新されたことを通知する
        if (EvOnUpdatedLocationAndRotation.IsBound()) {
            EvOnUpdatedLocationAndRotation.Broadcast();
        }

        PreLocation = CurrentLocation;
        PreRotation = CurrentRotation;
    }

}

// Called to bind functionality to input
void ATwinLinkWorldViewer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    // InputMappingContext追加、入力アクションに対しての関数のバインド処理はBluePrint側で行っている。
    // 下記のURLに倣って記述したがバインド出来なかったため
    // https://docs.unrealengine.com/5.2/ja/enhanced-input-in-unreal-engine/

    // 移動コンポーネントを利用する前に取得
    CharMovementComponent = GetCharacterMovement();
    check(CharMovementComponent);
    
    // 空を飛べるようにする
    CharMovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
}

void ATwinLinkWorldViewer::SetCityModel(AActor* Actor) {
    CityModel = Cast<APLATEAUInstancedCityModel>(Actor);

    // 地面を取得
    TArray<UPLATEAUCityObjectGroup*> CityObjGroups;
    CityModel->GetComponents<UPLATEAUCityObjectGroup>(CityObjGroups, true);
    for (const auto& CityObjGroup : CityObjGroups) {
        const auto bIsGround = CityObjGroup->GetName().StartsWith(TEXT("DEM_"), ESearchCase::IgnoreCase);
        if (bIsGround) {
            DemCityModel = CityObjGroup;
            break;
        }
    }
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

void ATwinLinkWorldViewer::AddFocusPoint() {
    if (!GetLocalViewingPlayerController())
        return;

    if (!CanReceivePlayerInput()) {
        return;
    }

    double Altitude = 0.0;
    if (DemCityModel != nullptr) {
        Altitude = DemCityModel->GetNavigationBounds().GetCenter().Z;
    }
    const auto Ground = FPlane(FVector::UpVector, Altitude); // 地面とみなす

    // 平面と光線が平行であった場合にヒットしない

    const auto CurrentRotation = GetNowCameraRotationOrDefault();
    const auto CurrentLocation = GetNowCameraLocationOrZero();

    const auto ExtraRadian = FMath::Sin(FMath::DegreesToRadians(LimitDegressFocusRotate));   // あまりにも地面と水平だと回転の半径が大きくなり視点の移動量がかなり大きくなる　操作しづらい
    bool bIsHit = CurrentRotation.Vector().Dot(Ground.GetNormal()) <= ExtraRadian;
    if (bIsHit == false) {
        return;
    }

    // 平面と光線のチェック
    FVector FocusPoint = FMath::RayPlaneIntersection(GetNowCameraLocationOrZero(), GetNowCameraRotationOrDefault().Vector(), Ground);

    // 0除算対策
    if (FocusPoint == CurrentLocation) {
        FocusPoint += GetActorForwardVector() * 0.001;
    }

    OffsetRotation = CurrentRotation;
    OffsetLength = FVector::Distance(FocusPoint, CurrentLocation);

    if (OffsetLength > LimitDistanceFocusPointToSelf) {
        return;
    }


    // 注視点を追加
    SingleFocusPoint = FocusPoint;

}

// カメラの前後移動
void ATwinLinkWorldViewer::MoveForward(const float Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    if (Controller && Value != 0.0f) {
        const auto Rotation = Controller->GetControlRotation();
        const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
        AddMovementInput(Direction, Value * CameraMovementSpeed * GetWorld()->GetDeltaSeconds());
    }
}

// カメラの横移動
void ATwinLinkWorldViewer::MoveRight(const float Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    if (Controller && Value != 0.0f) {
        const auto Rotation = Controller->GetControlRotation();
        const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
        AddMovementInput(Direction, Value * CameraMovementSpeed * GetWorld()->GetDeltaSeconds());
    }
}

// カメラの上下移動
void ATwinLinkWorldViewer::MoveUp(const float Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    if (Controller && Value != 0.0f) {
        const auto Direction = FVector::UpVector;
        AddMovementInput(Direction, Value * CameraMovementSpeed * GetWorld()->GetDeltaSeconds());
    }
}

// カメラの水平回転
// カメラの垂直回転
void ATwinLinkWorldViewer::TurnXY(const FVector2D Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    AddFocusPoint();
    if (SingleFocusPoint.has_value() == false) {
        AddControllerPitchInput(-Value.Y * CameraRotationSpeed);
        AddControllerYawInput(-Value.X * CameraRotationSpeed);
    }
    else {
        double Pitch = 0.0f;
        double Yaw = 0.0f;

        const auto Factor = 0.5f;
        Pitch = Value.Y * CameraRotationSpeed * Factor;
        Yaw = Value.X * CameraRotationSpeed * Factor;

        const auto FocusPoint = SingleFocusPoint.value();
        RotateAroundFocusPoint(this, FocusPoint, OffsetRotation, FRotator(Pitch, Yaw, 0.0), OffsetLength);

        //RotateAroundFocusPoints();
        ClearFocusPoint();

    }

}

void ATwinLinkWorldViewer::Click() {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    FHitResult HitResult;
    if (!GetLocalViewingPlayerController())
        return;

    if (EvOnClicked.IsBound()) {
        EvOnClicked.Broadcast();
    }

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

void ATwinLinkWorldViewer::InputedAny(bool bIsActive) {
    if (bIsActive) {
        NoInputProcessTime = 0.0f;

        if (CurrentAutoViewMode == ETwinLinkViewMode::FreeAutoView) {
            ActivateAutoViewControlButton(ETwinLinkViewMode::Manual);
        }
    }    
}

void ATwinLinkWorldViewer::ActivateAutoViewControl(ETwinLinkViewMode ViewMode) {
    const auto bIsChangeVal = ViewMode != CurrentAutoViewMode;
    if (bIsChangeVal == false)
        return;

    switch (ViewMode) {
    case ETwinLinkViewMode::FreeAutoView:
    {
        AddFocusPoint();
        const auto DemCityModelBounds = DemCityModel->GetNavigationBounds();
        const auto OffsetDistance = (DemCityModelBounds.GetExtent().X + DemCityModelBounds.GetExtent().Y) * 0.5;
        const auto FocusPoint = DemCityModelBounds.GetCenter();
        const auto CurrentRotator = GetNowCameraRotationOrDefault();
        const auto DefaultRotator = FRotator(-45.0f, CurrentRotator.Yaw, CurrentRotator.Roll);
        const auto DefaultLocation = FocusPoint - DefaultRotator.Vector() * OffsetDistance;
        AutoFreeViewControl.Init(FocusPoint, DefaultLocation, DefaultRotator);
    }
    break;
    case ETwinLinkViewMode::LimitedAutoView:
    {
        AddFocusPoint();
        AutoRotateViewProcessTime = 0.0f;
    }
    break;
    case ETwinLinkViewMode::Manual:
    {
        NoInputProcessTime = 0.0f;
        ClearFocusPoint();
        CurrentAutoViewMode = ETwinLinkViewMode::Manual;
        break;
    }
    }
    CurrentAutoViewMode = ViewMode;


}

void ATwinLinkWorldViewer::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) {
    Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
#if WITH_EDITOR
    // デバッグ表示させる
    ATwinLinkNavSystem::GetInstance(GetWorld())->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
#endif
}

bool ATwinLinkWorldViewer::CanReceivePlayerInput() {
    return CurrentAutoViewMode == ETwinLinkViewMode::Manual;
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

bool ATwinLinkWorldViewer::MoveInfo::IsCompleted() {
    return PassSec >= MoveSec || MoveSec <= 0.f;
}

void ATwinLinkWorldViewer::ClearFocusPoint() {
    SingleFocusPoint = std::nullopt;
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

void ATwinLinkWorldViewer::FAutoFreeViewControl::Init(const FVector& InFocusPoint, const FVector& InOffsetLocation, const FRotator& InOffsetRotator) {
    FocusPoint = InFocusPoint;
    OffsetLocation = InOffsetLocation;
    OffsetRotator = InOffsetRotator;
    OffsetDistance = FVector::Distance(FocusPoint, OffsetLocation);
    TimeCnt = 0.0f;
    SwitchIdxInUpdate = 0;
}

void ATwinLinkWorldViewer::FAutoFreeViewControl::Update(ATwinLinkWorldViewer* WorldViewer, float DeltaTime) {
    switch (SwitchIdxInUpdate) {
    case 0: // Offsetへの移動設定
        WorldViewer->SetLocation(OffsetLocation, OffsetRotator, MoveSec);
        SwitchIdxInUpdate++;
        break;
    case 1: // 移動中
        TimeCnt += DeltaTime;
        if (TimeCnt > MoveSec) {
            SwitchIdxInUpdate++;
            TimeCnt = 0.0f;
        }
        break;
    case 2: // 移動後、回転する
        TimeCnt += DeltaTime;
        RotateAroundFocusPoint(WorldViewer, FocusPoint,
            OffsetRotator, FRotator(0.0, WorldViewer->AutoRotationSpeed * TimeCnt, 0.0), OffsetDistance);
        break;
    default:
        break;
    }
}

