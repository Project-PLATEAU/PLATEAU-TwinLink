// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include <optional>

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TwinLinkWorldViewer.generated.h"

// 位置、姿勢情報が更新された
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FDelUpdatedLocationAndRotation);

// クリックイベントの取得
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FDelClickViewPort, FHitResult, HitResult);
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FDelCanceledClickFacility);
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FDelClicked);

// 任意のオブジェクトをクリックしたときのイベント
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FOnAnyObjectClicked, const FHitResult&);

/**
 * @brief 自動閲覧モードの種類
*/
UENUM(BlueprintType)
enum class ETwinLinkViewMode : uint8 {
    Undefind = 0,
    FreeAutoView = 0x01,        // 放置していていたらこれになる　待機画面的なところ
    LimitedAutoView = 0x02,     // 自動回転ボタンを押したらこれになる　望んだ注視点を中心に動く
    Manual = 0x04,              // 自動閲覧を行っていない 
    ManualWalk = 0x08,          // 自動閲覧を行っていない 歩行モード
};

// ViewModeが変更された
DECLARE_EVENT_OneParam(ATwinLinkWorldViewer, FDelOnChangedViewMode, ETwinLinkViewMode);

/**
 * @brief 視点操作機能を提供するカメラを搭載したキャラクタークラス
*/
UCLASS()
class TWINLINK_API ATwinLinkWorldViewer : public ACharacter {
    GENERATED_BODY()
public:
    /*
 * EntranceLocatorの参照ノード. EntranceLocatorはSingleton前提
 */
    class FInputControlNode {
    public:
        FInputControlNode() = default;
        virtual ~FInputControlNode();
        /*
         * @brief : 入力を制限するかどうか
         */
        virtual bool IsInputRestricted() const = 0;

        /*
         * @brief : 親のWorldViewerを設定する
         */
        void SetParent(TWeakObjectPtr<ATwinLinkWorldViewer> V);

        /*
         * @brief : リンクを削除
         */
        void Reset();
    private:
        TWeakObjectPtr<ATwinLinkWorldViewer> Parent = nullptr;
    };
public:
    // Sets default values for this character's properties
    ATwinLinkWorldViewer();

    static TWeakObjectPtr<ATwinLinkWorldViewer> GetInstance(UWorld* World);

    static void ChangeTwinLinkViewMode(UWorld* World, ETwinLinkViewMode ViewMode);

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
     * @brief CityModelの設定を行う
     * @param Actor
    */
    UFUNCTION(BlueprintCallable, Category = "Movement")
        void SetCityModel(AActor* Actor);

    /**
     * @brief 移動先の設定
     * 現在は指定した先に瞬間移動する
     * 後から高速移動に切り替えるかも
     * @param Position
     * @param Rotation
    */
    void SetLocation(const FVector& Position, const FRotator& Rotation, float MoveSec = 0.f);
    void SetLocation(const FVector& Position, const FVector& RotationEuler, float MoveSec = 0.f);

    /**
     * @brief 壁の中か確認する
     * @param Position 
     * @return 
    */
    bool IsInTheWall(const FVector& Position) const;

    /**
     * @brief 配置可能な位置を取得することを試みる
     * @param NewPosition 
     * @param TargetPosition 
     * @return 成功時 true
    */
    bool TryGetDeployPosition(FVector* const NewPosition, const FVector& TargetPosition);

    /**
     * @brief 指定位置に配置するように移動する
     * @param TargetPosition 
     * @param Rotation 
    */
    void Deploy(const FVector& TargetPosition, const FRotator& Rotation);


    /**
     * @brief 移動先の設定
     * 現在は指定した先に瞬間移動する
     * 後から高速移動に切り替えるかも
     * @param Position
     * @param LookAt
     * @param bForce : trueの時は即座に適用
    */
    void SetLocationLookAt(const FVector& Position, const FVector& LookAt, float MoveSec = 0.f);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        FVector GetNowCameraLocationOrZero() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
        FRotator GetNowCameraRotationOrDefault() const;



    /**
     * @brief 自動回転ボタンを有効にする
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void ActivateAutoViewControlButton(ETwinLinkViewMode Mode);

    bool IsWalkMode();

    /*
     * @brief : 入力制御用のノードを追加する
     */
    void AddInputControlNode(FInputControlNode* Node);

    /*
     * @brief : 入力制御用ノードを削除
     */
    void RemoveInputControlNode(FInputControlNode* Node);
private:
    void ATwinLinkWorldViewer::SetLocationImpl(const FVector& Position, const FRotator& Rotation);
    std::optional<FVector> CalcFocusPoint();

public:
    // 位置、姿勢が更新された
    FDelUpdatedLocationAndRotation EvOnUpdatedLocationAndRotation;

    // クリック
    FDelClickViewPort EvOnClickedFacility;
    FDelCanceledClickFacility EvOnCanceledClickFacility;
    FDelClicked EvOnClicked;

    // 任意のオブジェクトをクリックしたときの処理
    FOnAnyObjectClicked EvOnAnyObjectClicked;

    // ViewModeが変更された
    FDelOnChangedViewMode EvOnChangedViewMode;
public:
    UFUNCTION(BlueprintCallable, Category = "Movement")
        void MoveForward(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void MoveForwardOnWorldSpace(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void MoveRight(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void MoveUp(const float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
        void TurnXY(const FVector2D Value);

private:
    UFUNCTION(BlueprintCallable, Category = "Movement")
        void Click();


    UFUNCTION(BlueprintCallable, Category = "TwinLink Auto")
        void InputedAny(bool bIsActive);


    UFUNCTION(BlueprintCallable, Category = "TwinLink Auto")
        void ActivateAutoViewControl(ETwinLinkViewMode ViewMode);

public:
    virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

    /**
     * @brief 
     * @param Idx 0 飛行　1 歩き 
     * @param Multiply 
    */
    void SetWorldViewerMovementMode(int Idx, float Multiply);

    void MoveForwardImpl(float Value, bool bUseWorldSpace = false);
    void MoveRightImpl(float Value);
    void TurnXYImpl(const FVector2D Value, bool bUseFocusPoint = false);

private:
    ETwinLinkViewMode Or(ETwinLinkViewMode a, ETwinLinkViewMode b) {
        return static_cast<ETwinLinkViewMode>(static_cast<int>(a) | static_cast<int>(b));
    }
    ETwinLinkViewMode And(ETwinLinkViewMode a, ETwinLinkViewMode b) {
        return static_cast<ETwinLinkViewMode>(static_cast<int>(a) & static_cast<int>(b));
    }
    bool IsZero(ETwinLinkViewMode a) {
        return static_cast<int>(a) == 0;
    }

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

    /* 飛行中の減速率 */
    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement", meta = (ClampMin = "0", UIMin = "0"))
        float BrakingDecelerationFlying = 81920.0f;



    /* このプロパティ値で他のシステムの値を上書きする */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "TwinLink View Movement")
        bool bOverrideOtherSystemValues = true;

    /*  */

    /* 自動閲覧モードの回転の速度 */
    UPROPERTY(EditAnywhere, Category = "TwinLink Auto")
        float AutoRotationSpeed = 0.5f;

    /* 自動閲覧モードの開始するまでの時間　秒 */
    UPROPERTY(EditAnywhere, Category = "TwinLink Auto")
        float LimitBeginAutoViewControlModeTime = 60.0 * 5.0;

    /* 注視点回転時の角度の制限　地面との平行を0として下方向を向くほど数値が小さくなる */
    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement", meta = (ClampMin = "0", UIMin = "0", ForceUnits = "deg"))
        float LimitAngleFocusPointRotate = -10.0f;

    /* 注視点回転時の距離の制限 */
    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement", meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
        float LimitDistanceFocusPointToSelf = 400000.0f;

    UPROPERTY(EditAnywhere, Category = "TwinLink View Movement")
        float WalkingCameraMovementSpeedFactor = 0.001f;

    UPROPERTY()
        UCharacterMovementComponent* CharMovementComponent;

    bool bIsSelectingFacility;

    struct Transform {
        FVector Location;
        FRotator Rotation;
    };

    struct MoveInfo {
        // 開始地点
        Transform From;
        // 目的地店
        Transform To;
        // 移動にかかる時間
        float MoveSec = 0.f;
        // 現在の経過時間
        float PassSec = 0.f;
        bool Update(float DeltaSec, FVector& OutLocation, FRotator& OutRotation);
        bool IsCompleted();
    };

    std::optional<MoveInfo> TargetTransform;

    std::optional<FVector> PreLocation;
    std::optional<FRotator> PreRotation;

    /** 前フレームのマウス座標 **/
    FVector2D PreMousePosition;

    /** 何の入力もない時間 **/
    float NoInputProcessTime;
    /** 移動入力が行われたか **/
    bool bInputedMovement;

    /** 放置状態関係のメンバーや処理をまとめた構造体 **/
    struct FAutoFreeViewControl {
        void Init(const FVector& InFocusPoint,
            const FVector& InOffsetLocation,
            const FRotator& InOffsetRotator);
        void Update(ATwinLinkWorldViewer* WorldViewer, float DeltaTime);

        /** 0,1... Update()で行う処理 **/
        int SwitchIdxInUpdate;
        FVector FocusPoint;
        FVector OffsetLocation;
        double OffsetDistance;
        FRotator OffsetRotator;
        float TimeCnt;
        const float MoveSec = 0.5f;
    } AutoFreeViewControl;

    class IViewModeState {
    public:
        void Init(ATwinLinkWorldViewer* WorldViewer) { Viewer = WorldViewer; }
        virtual void Tick(float DeltaTime) {};
        virtual void ActivateAutoViewControl() {};
        virtual void InputedAny(){};
        virtual bool CanReceivePlayerInput() { return false; };
        virtual void MoveForward(float Value) {};
        virtual void MoveRight(float Value) {};
        virtual void TurnXY(const FVector2D Value) {};

    protected:
        ATwinLinkWorldViewer* Viewer;
    };

    class FreeAutoViewState : public IViewModeState {
    public:
        // Inherited via IViewModeState
        void Tick(float DeltaTime) override;
        void ActivateAutoViewControl() override;
        void InputedAny() override;
        bool CanReceivePlayerInput() override { return false; };

    private:
        FAutoFreeViewControl AutoFreeViewControl;
    };

    class LimitedAutoViewState : public IViewModeState {
    public:
        // Inherited via IViewModeState
        void Tick(float DeltaTime) override;
        void ActivateAutoViewControl() override;
        bool CanReceivePlayerInput() override { return false; };

    private:
        float AutoRotateViewProcessTime;
    };

    class ManualState : public IViewModeState {
    public:
        // Inherited via IViewModeState
        void ActivateAutoViewControl() override;
        bool CanReceivePlayerInput() override { return true; };
        void MoveForward(float Value) override;
        void MoveRight(float Value) override;
        void TurnXY(const FVector2D Value) override;

    };

    class ManualWalkState : public IViewModeState {
    public:
        // Inherited via IViewModeState
        void Tick(float DeltaTime) override;
        void ActivateAutoViewControl() override;
        bool CanReceivePlayerInput() override { return true; };
        void MoveForward(float Value) override;
        void MoveRight(float Value) override;
        void TurnXY(const FVector2D Value) override;

    private:
        // 移動入力が継続して行われているかつ座標を変化していない時間
        float AnyInputMovementAndNotMovementProcessTime;

    };

    class ViewModeStateMachine {
    public:
        ViewModeStateMachine() 
            :CurrentState(nullptr) {}

        bool IsDiffrentViewMode(ETwinLinkViewMode ViewMode) {
            return ViewMode != CurrentAutoViewMode;
        }

        bool CanReceivePlayerInput() { return CurrentState->CanReceivePlayerInput(); };

        void Init(ATwinLinkWorldViewer* WorldViewer);
        bool IsInited() { CurrentState != nullptr; }

        void Tick(float DeltaTime) { CurrentState->Tick(DeltaTime); };
        void ActivateAutoViewControl(ETwinLinkViewMode ViewMode) { 
            ChangeMode(ViewMode);
            CurrentState->ActivateAutoViewControl(); 
        };

        void InputedAny() { CurrentState->InputedAny(); };

        void MoveForward(float Value) { CurrentState->MoveForward(Value); }
        void MoveRight(float Value) { CurrentState->MoveRight(Value); }
        void TurnXY(const FVector2D Value) { CurrentState->TurnXY(Value); }

        ETwinLinkViewMode GetCurrentMode() { return CurrentAutoViewMode; }
    private:
        void ChangeMode(ETwinLinkViewMode ViewMode);

    private:
        IViewModeState* CurrentState;
        ETwinLinkViewMode CurrentAutoViewMode;

        FreeAutoViewState FreeAutoViewStateInst;
        LimitedAutoViewState LimitedAutoViewStateInst;
        ManualState ManualStateInst;
        ManualWalkState ManualWalkStateInst;
    } StateMachine;


    /** 実際に利用する移動速度 **/
    float CurrentCameraMovementSpeed;

private:
    /**
     * @brief 注視点を中心に回転する
    */
    void RotateAroundFocusPoints();

    /**
     * @brief プレイヤーの入力を受け付けるか
     * @return
    */
    bool CanReceivePlayerInput();

    /**
     * @brief 注視点から現在のカメラの座標までの距離
     * @return
    */
    double CalcOffsetLength(std::optional<FVector> FocusPoint);

    /**
     * @brief 
     * @return 
    */
    FCollisionShape CreateCollisionShape() const;

    /*
     * @brief : 入力制限用ノードリスト
     */
    TArray<FInputControlNode*> InputControlNodes;
};
