// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkWorldViewer.h"

#include "TwinLink.h"

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
    void RotateAroundFocusPoint(ATwinLinkWorldViewer* Self, const FVector& FocusPoint, const FRotator& BaseRotator, const FRotator& AddRotator, double OffsetDistance, float XAngleLimitMax = 85) {
        const auto LimitXMax = XAngleLimitMax;
        const auto LimitXMin = -85;
        const auto OffsetRotationNormalized = BaseRotator.GetNormalized();
        const auto RotatorX = FMath::Clamp(OffsetRotationNormalized.Pitch + AddRotator.Pitch, LimitXMin, LimitXMax);
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

ATwinLinkWorldViewer::FInputControlNode::~FInputControlNode() {
    Reset();
}

void ATwinLinkWorldViewer::FInputControlNode::SetParent(TWeakObjectPtr<ATwinLinkWorldViewer> V) {
    Parent = V;
}

void ATwinLinkWorldViewer::FInputControlNode::Reset() {
    if (Parent.IsValid())
        Parent->RemoveInputControlNode(this);
    Parent = nullptr;
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

void ATwinLinkWorldViewer::ChangeTwinLinkViewMode(UWorld* World, ETwinLinkViewMode ViewMode) {
    TObjectPtr<ATwinLinkWorldViewer> Viewer = GetInstance(World).Get();

    if (Viewer) {
        if (!Viewer->StateMachine.IsInited())
            return;
        Viewer->ActivateAutoViewControlButton(ViewMode);
    }
}

// Called when the game starts or when spawned
void ATwinLinkWorldViewer::BeginPlay() {
    Super::BeginPlay();

    TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = GetComponentByClass<UCapsuleComponent>();
    _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    StateMachine.Init(this);
    ActivateAutoViewControlButton(ETwinLinkViewMode::FreeAutoView);

    NoInputProcessTime = LimitBeginAutoViewControlModeTime - 10.0f; // デフォルトを放置状態スタートにする

    //
    CurrentCameraMovementSpeed = CameraMovementSpeed;
}

// Called every frame
void ATwinLinkWorldViewer::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // マウスの移動を検知する
    const auto PlayerController = GetLocalViewingPlayerController();
    const auto CurrentMousePosition = ATwinLinkPlayerController::GetMousePosition(PlayerController).value_or(PreMousePosition);
    if (FVector2D::DistSquared(PreMousePosition, CurrentMousePosition) > 0.01 * 0.01) {
        InputedAny(true);
    }

    // 管理者モードでは放置状態に移行しない
    if (FTwinLinkModule::Get().IsAdminModeEnabled() == false) {
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
            if (StateMachine.IsInited()) {
                ActivateAutoViewControlButton(ETwinLinkViewMode::FreeAutoView);
                return;
            }
        }
    }
    else {
        NoInputProcessTime = 0.0f;
    }

    StateMachine.Tick(DeltaTime);

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
    if (PreLocation.value_or(CurrentLocation) != CurrentLocation ||
        PreRotation.value_or(CurrentRotation) != CurrentRotation) {
        // 更新されたことを通知する
        if (EvOnUpdatedLocationAndRotation.IsBound()) {
            EvOnUpdatedLocationAndRotation.Broadcast();
        }
    }

    // 前フレーム座標を更新する
    PreLocation = CurrentLocation;
    PreRotation = CurrentRotation;

    // 前フレームのマウスの座標を更新する
    PreMousePosition = CurrentMousePosition;

    // 移動入力の検出状態を元に戻す
    bInputedMovement = false;

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
}

std::optional<FVector> ATwinLinkWorldViewer::CalcFocusPoint() {
    if (!GetLocalViewingPlayerController())
        return std::nullopt;

    std::optional<FVector> Ret;

    double Altitude = 0.0;
    if (DemCityModel != nullptr) {
        Altitude = DemCityModel->GetNavigationBounds().GetCenter().Z;
    }
    const auto Ground = FPlane(FVector::UpVector, Altitude); // 地面とみなす

    // 平面と光線が平行であった場合にヒットしない

    const auto CurrentRotation = GetNowCameraRotationOrDefault();
    const auto CurrentLocation = GetNowCameraLocationOrZero();

    if (LimitAngleFocusPointRotate < CurrentRotation.Pitch) {
        return std::nullopt;
    }

    const auto DotRotVecAndGroundNormal = CurrentRotation.Vector().Dot(Ground.GetNormal());
    // 地面と平行 ロックしてしまうため調整 仮の注視点を設定
    if (DotRotVecAndGroundNormal * DotRotVecAndGroundNormal < 0.001 * 0.001) {
        return CurrentLocation + CurrentRotation.Vector() * (LimitDistanceFocusPointToSelf - 1);
    }

    // 地面よりも下にいる　この状態でRayを飛ばすと逆方向の地面にヒットするためスキップ
    if (CurrentLocation.Z <= Altitude) {
        return std::nullopt;
    }

    // 平面と光線のチェック
    FVector FocusPoint = FMath::RayPlaneIntersection(CurrentLocation, CurrentRotation.Vector(), Ground);

    // 0除算対策
    if (FocusPoint == CurrentLocation) {
        FocusPoint += GetActorForwardVector() * 0.001;
    }

    const auto OffsetLength = CalcOffsetLength(FocusPoint);
    if (OffsetLength > LimitDistanceFocusPointToSelf) {
        return std::nullopt;
    }


    // 注視点を追加
    return FocusPoint;
}

// カメラの前後移動
void ATwinLinkWorldViewer::MoveForward(const float Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    if (Controller && Value != 0.0f) {
        StateMachine.MoveForward(Value);
    }
}

void ATwinLinkWorldViewer::MoveForwardOnWorldSpace(const float Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    if (Controller && Value != 0.0f) {
        MoveForwardImpl(Value, true);
    }
}

// カメラの横移動
void ATwinLinkWorldViewer::MoveRight(const float Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }

    if (Controller && Value != 0.0f) {
        StateMachine.MoveRight(Value);
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
        AddMovementInput(Direction, Value * CurrentCameraMovementSpeed);
    }
}

// カメラの水平回転
// カメラの垂直回転
void ATwinLinkWorldViewer::TurnXY(const FVector2D Value) {
    // 
    if (!CanReceivePlayerInput()) {
        return;
    }
    StateMachine.TurnXY(Value);
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
        StateMachine.InputedAny();
    }
}

void ATwinLinkWorldViewer::ActivateAutoViewControl(ETwinLinkViewMode ViewMode) {
    const auto bIsChangeVal = StateMachine.IsDiffrentViewMode(ViewMode);
    if (bIsChangeVal == false)
        return;

    if (CityModel == nullptr)
        return;
    if (DemCityModel == nullptr)
        return;
    
    StateMachine.ActivateAutoViewControl(ViewMode);
    EvOnChangedViewMode.Broadcast(ViewMode);
}

void ATwinLinkWorldViewer::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) {
    Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
#if WITH_EDITOR
    // デバッグ表示させる
    ATwinLinkNavSystem::GetInstance(GetWorld())->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
#endif
}

void ATwinLinkWorldViewer::SetWorldViewerMovementMode(int Idx, float Multiply) {
    switch (Idx) {
    case 0:
        CurrentCameraMovementSpeed = CameraMovementSpeed;
        break;
    case 1:
        CurrentCameraMovementSpeed = CameraMovementSpeed * WalkingCameraMovementSpeedFactor;
        break;
    }

    CurrentCameraMovementSpeed *= Multiply;
}

void ATwinLinkWorldViewer::MoveForwardImpl(float Value, bool bUseWorldSpace) {
    FVector Direction;
    const auto Rotation = Controller->GetControlRotation();
    if (bUseWorldSpace == false) {
        Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
    }
    else {
        Direction = FRotator(0, Rotation.Yaw, 0).Vector();
    }
    AddMovementInput(Direction, Value * CurrentCameraMovementSpeed);
    bInputedMovement = true;
}

void ATwinLinkWorldViewer::MoveRightImpl(float Value) {
    const auto Rotation = Controller->GetControlRotation();
    const auto Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
    AddMovementInput(Direction, Value * CurrentCameraMovementSpeed);
    bInputedMovement = true;
}

void ATwinLinkWorldViewer::TurnXYImpl(const FVector2D Value, bool bUseFocusPoint) {
    if (bUseFocusPoint) {
        const auto FocusPoint = CalcFocusPoint();
        if (FocusPoint.has_value()) {
            double Pitch = 0.0f;
            double Yaw = 0.0f;

            const auto Factor = 100.0f;
            Pitch = Value.Y * CameraRotationSpeed * Factor * GetWorld()->DeltaTimeSeconds;
            Yaw = Value.X * CameraRotationSpeed * Factor * GetWorld()->DeltaTimeSeconds;

            RotateAroundFocusPoint(this, FocusPoint.value(), GetNowCameraRotationOrDefault(), FRotator(Pitch, Yaw, 0.0), CalcOffsetLength(FocusPoint), LimitAngleFocusPointRotate);
        }
    }
    else {
        AddControllerPitchInput(-Value.Y * CameraRotationSpeed);
        AddControllerYawInput(-Value.X * CameraRotationSpeed);
    }

}

bool ATwinLinkWorldViewer::CanReceivePlayerInput() {
    if (StateMachine.CanReceivePlayerInput() == false)
        return false;

    const auto bIsRestricted = InputControlNodes.ContainsByPredicate([](const FInputControlNode* X) { return X && X->IsInputRestricted(); });
    return bIsRestricted == false;
}

double ATwinLinkWorldViewer::CalcOffsetLength(std::optional<FVector> FocusPoint) {
    const auto CurrentLocation = GetNowCameraLocationOrZero();
    const auto CurrentRotation = GetNowCameraRotationOrDefault();
    const auto OffsetLength = FVector::Distance(FocusPoint.value_or(CurrentLocation + CurrentRotation.Vector()), CurrentLocation);
    return OffsetLength;
}

FCollisionShape ATwinLinkWorldViewer::CreateCollisionShape() const {
    const auto Capsule = GetCapsuleComponent();
    const auto Radius = Capsule->GetScaledCapsuleRadius();
    const auto HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

    FCollisionShape Shape;
    Shape.SetCapsule(Radius, HalfHeight);
    return Shape;
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

bool ATwinLinkWorldViewer::IsWalkMode() {
    return StateMachine.GetCurrentMode() == ETwinLinkViewMode::ManualWalk;
}

void ATwinLinkWorldViewer::AddInputControlNode(FInputControlNode* Node) {
    if (!Node)
        return;

    InputControlNodes.Add(Node);
    Node->SetParent(this);
}

void ATwinLinkWorldViewer::RemoveInputControlNode(FInputControlNode* Node) {
    if (!Node)
        return;
    // Nodeをリストから削除(念のため多重登録が無いかも見る)
    InputControlNodes.RemoveAll([Node](FInputControlNode* X) { return X == Node; });
}

void ATwinLinkWorldViewer::SetLocationImpl(const FVector& Position, const FRotator& Rotation) {
    CharMovementComponent->StopMovementImmediately();   // 現在の移動速度を０にする
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

bool ATwinLinkWorldViewer::IsInTheWall(const FVector& Position) const {
    FCollisionShape CollisionShape = CreateCollisionShape();

    FHitResult HitResult;
    const auto TestDistance = 100000.0f;  // 1km  1km以内に自身の位置と反対方向を向いた壁が無ければ判定が取れないので増やす
    static const FVector EndDirs[] = { 
        FVector::UpVector, FVector::DownVector,
        FVector::RightVector, FVector::LeftVector,
        FVector::ForwardVector, FVector::BackwardVector
    };
    for (int i = 0; i < sizeof(EndDirs) / sizeof(FVector); i++) {
        const auto End = Position + EndDirs[i] * TestDistance;
        FVector StartReturnLineTrace = End;

        //const auto bIsHit = GetWorld()->SweepSingleByChannel(HitResult, Position, End, FQuat::Identity, ECollisionChannel::ECC_Visibility, CollisionShape);
        FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
        FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam;
        const auto bIsHit = GetWorld()->LineTraceSingleByChannel(
            HitResult, Position, End, ECollisionChannel::ECC_Visibility,
            Params, ResponseParams);    // 自身の方向を向いた壁を検出
        if (bIsHit) {
            StartReturnLineTrace = HitResult.ImpactPoint;
        }

        bool bIsHitRWall = GetWorld()->LineTraceTestByChannel(StartReturnLineTrace, Position, ECollisionChannel::ECC_Visibility);
        if (bIsHitRWall) {
            //// 壁の裏面かチェック
            //const float DebugRadius = 100.0f;
            //const float DebugLife = 30.0f;
            //DrawDebugDirectionalArrow(
            //    GetWorld(), HitResult.TraceStart, HitResult.ImpactPoint,
            //    DebugRadius * 100, FColor::Red, false, DebugLife, UINT8_MAX);
            //DrawDebugDirectionalArrow(
            //    GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint + HitResult.ImpactNormal * 1000,
            //    DebugRadius * 100, FColor::Blue, false, DebugLife, UINT8_MAX);
            //DrawDebugSphere(
            //    GetWorld(), HitResult.TraceStart, DebugRadius, 16,
            //    FColor::Red, false, DebugLife, UINT8_MAX);
            //DrawDebugSphere(
            //    GetWorld(), HitResult.ImpactPoint, DebugRadius, 16,
            //    FColor::Blue, false, DebugLife, UINT8_MAX);
            
            return true;
        }
    }
    return false;
}

bool ATwinLinkWorldViewer::TryGetDeployPosition(FVector* const NewPosition, const FVector& TargetPosition) {
    const auto CurrentLocation = GetNowCameraLocationOrZero();

    const auto VecViewerToImpactPoint = TargetPosition - CurrentLocation;
    
    const auto Capsule = GetCapsuleComponent();
    const auto Radius = Capsule->GetScaledCapsuleRadius();
    const auto HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    const auto Extent = FMath::Max(Radius, HalfHeight) + 1;
    const auto VecViewerToImpactPointLength = VecViewerToImpactPoint.Length();
    const auto DistanceToTargetLocation = FMath::Max(VecViewerToImpactPointLength - Extent, 0);   // 現在位置よりは下がらないようにする
    const auto VecViewerToImpactPointNormal = VecViewerToImpactPoint.GetUnsafeNormal();
    const auto TargetLocation = CurrentLocation + VecViewerToImpactPointNormal * DistanceToTargetLocation;

    FCollisionShape CollisionShape;
    const auto ExtraScale = 2.0f;
    CollisionShape.SetCapsule(Radius * ExtraScale, HalfHeight * ExtraScale);

    // 配置可能な位置可能な位置に配置する
    // 指定座標から元に位置に向かって配置可能か順にテストする
    bool bIsDeployable = false;
    int SweepTestCnt = 0;
    const auto MaxSweepTestCnt = 10;
    FVector AvoidOffset;
    FVector RealTargetLocation;
    while (true) {
        // 試行回数の制限を超えた
        if (SweepTestCnt >= MaxSweepTestCnt)
            break;

        AvoidOffset = -VecViewerToImpactPointNormal * Extent * SweepTestCnt;
        RealTargetLocation = TargetLocation + AvoidOffset;
        const auto bIsOverlap = GetWorld()->OverlapAnyTestByChannel(
            RealTargetLocation,
            FQuat::Identity, ECollisionChannel::ECC_Visibility, CollisionShape);

        // 配置しても衝突しない
        if (bIsOverlap == false) {
            // 建物内じゃない
            if (IsInTheWall(RealTargetLocation) == false) {
                // 配置可能な位置を見つけた
                break;
            }
        }
        SweepTestCnt++;
    }

    // 試行回数以内に配置可能な位置を見つけた
    if (SweepTestCnt < MaxSweepTestCnt) {
        *NewPosition = RealTargetLocation;
        return true;
    }

    return false;
}

void ATwinLinkWorldViewer::Deploy(const FVector& TargetPosition, const FRotator& Rotation) {
    SetLocation(TargetPosition, Rotation, 0.5f);
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
        if (FVector::DistSquared(WorldViewer->GetNowCameraLocationOrZero(), WorldViewer->TargetTransform->To.Location) <= 0.01) {
            TimeCnt = MoveSec;
        }
        if (TimeCnt >= MoveSec) {
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

void ATwinLinkWorldViewer::ViewModeStateMachine::Init(ATwinLinkWorldViewer* WorldViewer) {
    FreeAutoViewStateInst.Init(WorldViewer);
    LimitedAutoViewStateInst.Init(WorldViewer);
    ManualStateInst.Init(WorldViewer);
    ManualWalkStateInst.Init(WorldViewer);

    ChangeMode(ETwinLinkViewMode::Manual);
}

void ATwinLinkWorldViewer::ViewModeStateMachine::ChangeMode(ETwinLinkViewMode ViewMode) {
    CurrentAutoViewMode = ViewMode;

    switch (ViewMode) {
    case ETwinLinkViewMode::FreeAutoView:
        CurrentState = &FreeAutoViewStateInst;
        break;
    case ETwinLinkViewMode::LimitedAutoView:
        CurrentState = &LimitedAutoViewStateInst;
        break;
    case ETwinLinkViewMode::Manual:
        CurrentState = &ManualStateInst;
        break;
    case ETwinLinkViewMode::ManualWalk:
        CurrentState = &ManualWalkStateInst;
        break;
    default:
        break;
    }
}

void ATwinLinkWorldViewer::FreeAutoViewState::Tick(float DeltaTime) {
    AutoFreeViewControl.Update(Viewer, DeltaTime);
}

void ATwinLinkWorldViewer::FreeAutoViewState::ActivateAutoViewControl() {
    TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = Viewer->GetComponentByClass<UCapsuleComponent>();
    _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    const auto DemCityModelBounds = DemCityModel->GetNavigationBounds();
    const auto OffsetDistance = (DemCityModelBounds.GetExtent().X + DemCityModelBounds.GetExtent().Y) * 0.5;
    const auto FocusPoint = DemCityModelBounds.GetCenter();
    const auto CurrentRotator = Viewer->GetNowCameraRotationOrDefault();
    const auto DefaultRotator = FRotator(-45.0f, CurrentRotator.Yaw, CurrentRotator.Roll);
    const auto DefaultLocation = FocusPoint - DefaultRotator.Vector() * OffsetDistance;
    AutoFreeViewControl.Init(FocusPoint, DefaultLocation, DefaultRotator);
}

void ATwinLinkWorldViewer::FreeAutoViewState::InputedAny() {
    Viewer->ActivateAutoViewControlButton(ETwinLinkViewMode::Manual);
}

void ATwinLinkWorldViewer::LimitedAutoViewState::Tick(float DeltaTime) {
    bool bCanAutoView = Viewer->TargetTransform.has_value() == false;
    if (bCanAutoView == false) {
        bCanAutoView = Viewer->TargetTransform.value().IsCompleted();
    }

    if (bCanAutoView) {
        const auto FocusPoint = Viewer->CalcFocusPoint();
        if (FocusPoint.has_value()) {
            RotateAroundFocusPoint(Viewer, FocusPoint.value(),
                Viewer->GetNowCameraRotationOrDefault(), 
                FRotator(0.0, Viewer->AutoRotationSpeed * DeltaTime, 0.0), 
                Viewer->CalcOffsetLength(FocusPoint), Viewer->LimitAngleFocusPointRotate);
        }
        else {
            Viewer->AddControllerYawInput(Viewer->AutoRotationSpeed * Viewer->CameraRotationSpeed);
        }
        AutoRotateViewProcessTime += DeltaTime;
    }
}

void ATwinLinkWorldViewer::LimitedAutoViewState::ActivateAutoViewControl() {
    AutoRotateViewProcessTime = 0.0f;
    TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = Viewer->GetComponentByClass<UCapsuleComponent>();
    _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATwinLinkWorldViewer::ManualState::ActivateAutoViewControl() {
    Viewer->NoInputProcessTime = 0.0f;
    TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = Viewer->GetComponentByClass<UCapsuleComponent>();
    _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATwinLinkWorldViewer::ManualState::MoveForward(float Value) {
    Viewer->MoveForwardImpl(Value);
}

void ATwinLinkWorldViewer::ManualState::MoveRight(float Value) {
    Viewer->MoveRightImpl(Value);
}

void ATwinLinkWorldViewer::ManualState::TurnXY(const FVector2D Value) {
    Viewer->TurnXYImpl(Value, true);
}

void ATwinLinkWorldViewer::ManualWalkState::Tick(float DeltaTime) {
    const auto CurrentLocation = Viewer->GetNowCameraLocationOrZero();
    const auto _PreLocation = Viewer->PreLocation.value_or(FVector::Zero());
    // 0とみなす距離
    const auto SqrDistanceZero = 1.0;

    // Stuckしていると判断する時間
    const auto CriterionStuckTime = 5.0f;

    // スタックしていないw
    if (AnyInputMovementAndNotMovementProcessTime < CriterionStuckTime) {
        // 入力がある状態を検出
        if (Viewer->bInputedMovement) {
            if (FVector::DistSquared(CurrentLocation, _PreLocation) < SqrDistanceZero) {
                AnyInputMovementAndNotMovementProcessTime += DeltaTime;

                // スタックした
                if (AnyInputMovementAndNotMovementProcessTime > CriterionStuckTime) {
                    TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = Viewer->GetComponentByClass<UCapsuleComponent>();
                    _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
        }
        else {
            AnyInputMovementAndNotMovementProcessTime = 0.0f;
        }
    }
    // スタック中
    else {
        // 動けた
        if (FVector::DistSquared(CurrentLocation, _PreLocation) > SqrDistanceZero) {
            AnyInputMovementAndNotMovementProcessTime = CriterionStuckTime * 0.8f;   // 連続してスタックする可能性があるのでカウントを完全には回復しない
            TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = Viewer->GetComponentByClass<UCapsuleComponent>();
            _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);                        
        }
    }

}

void ATwinLinkWorldViewer::ManualWalkState::ActivateAutoViewControl() {
    Viewer->NoInputProcessTime = 0.0f;
    AnyInputMovementAndNotMovementProcessTime = 0.0f;
    TWeakObjectPtr<UCapsuleComponent> _CapsuleComponent = Viewer->GetComponentByClass<UCapsuleComponent>();
    _CapsuleComponent.Get()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ATwinLinkWorldViewer::ManualWalkState::MoveForward(float Value) {
    Viewer->MoveForwardImpl(Value, true);
}

void ATwinLinkWorldViewer::ManualWalkState::MoveRight(float Value) {
    Viewer->MoveRightImpl(Value);
}

void ATwinLinkWorldViewer::ManualWalkState::TurnXY(const FVector2D Value) {
    Viewer->TurnXYImpl(FVector2D(-Value.X, Value.Y), false);
}
