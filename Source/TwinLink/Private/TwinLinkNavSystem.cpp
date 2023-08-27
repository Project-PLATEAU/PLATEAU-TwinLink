#include "TwinLinkNavSystem.h"
#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h" 

namespace {
    template<class T>
    auto CreateChildActor(ATwinLinkNavSystem* self, TSubclassOf<T> Class, const TCHAR* Name) -> T* {
        FActorSpawnParameters Params;
        Params.Owner = self;
        Params.Name = FName(Name);
        const auto Ret = self->GetWorld()->SpawnActor<T>(Class, Params);
        if (!Ret)
            return Ret;
        Ret->SetActorLabel(FString(Name));
        Ret->AttachToActor(self, FAttachmentTransformRules::KeepWorldTransform, Name);
        return Ret;
    };

    template<class T, class F>
    void ForeachChildActor(AActor* Self, F&& Func) {
        if (!Self)
            return;
        for (auto& Child : Self->Children) {
            if (auto Target = Cast<T>(Child))
                Func(Target);

            ForeachChildActor<T>(Child, std::forward<F>(Func));
        }
    }
}


ATwinLinkNavSystem::ATwinLinkNavSystem() {
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

bool ATwinLinkNavSystem::CreateFindPathRequest(const FVector& Start, const FVector& End, FPathFindingQuery& OutRequest) const {
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return false;
    OutRequest = FPathFindingQuery();
    OutRequest.StartLocation = Start;
    OutRequest.EndLocation = End;
    // #NOTE : ナビメッシュは車と人で区別しないということなのでエージェントは設定しない
    //query.NavAgentProperties.AgentHeight = 100;
    //query.NavAgentProperties.AgentRadius = 10;
    OutRequest.NavData = NavSys->GetDefaultNavDataInstance();
    OutRequest.Owner = this;
    return true;
}

TwinLinkNavSystemFindPathInfo ATwinLinkNavSystem::RequestFindPath(const FVector& Start, const FVector& End) const {

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FPathFindingQuery Query;
    CreateFindPathRequest(Start, End, Query);
    TwinLinkNavSystemFindPathInfo Ret = TwinLinkNavSystemFindPathInfo(NavSys->FindPathSync(Query, EPathFindingMode::Regular));
    if (Ret.PathFindResult.IsSuccessful()) {
        auto GetHeightCheckedPos = [this](FVector Pos) {
            auto HeightMax = Pos;
            HeightMax.Z = DemCollisionAabb.Max.Z + 1;
            auto HeightMin = Pos;
            HeightMin.Z = DemCollisionAabb.Min.Z - 1;
            FHitResult HeightResult;

            if (GetWorld()->LineTraceSingleByChannel(HeightResult, HeightMax, HeightMin, DemCollisionChannel))
                Pos.Z = HeightResult.Location.Z + 1;
            return Pos;
        };

        auto& points = Ret.PathFindResult.Path->GetPathPoints();
        FVector LastPos = FVector::Zero();
        float ModLength = 0.f;
        for (auto i = 0; i < points.Num(); ++i) {
            auto& p = points[i];
            auto Pos = GetHeightCheckedPos(p.Location);
            if (i > 0) {
                auto Dir = (Pos - LastPos);
                auto Length = ModLength + Dir.Length();
                Dir.Normalize();
                auto CheckNum = static_cast<int32>((Length - ModLength) / FindPathHeightCheckInterval);
                for (auto I = 1; I <= CheckNum; ++I) {
                    auto P = GetHeightCheckedPos(LastPos + Dir * FindPathHeightCheckInterval);
                    Ret.HeightCheckedPoints.Add(P);
                    LastPos = P;
                }
            }
            Ret.HeightCheckedPoints.Add(Pos);
            LastPos = Pos;
        }
    }
    return Ret;
}

void ATwinLinkNavSystem::RequestFindPath() {
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return;
    auto Start = GetStartPoint();
    Start.Z = 0;
    FNavLocation OutStart;
    if (NavSys->ProjectPointToNavigation(Start, OutStart, FVector::One() * 100))
        Start = OutStart;

    auto Dest = GetDestPoint();
    Dest.Z = 0;
    FNavLocation OutDest;
    if (NavSys->ProjectPointToNavigation(Dest, OutDest, FVector::One() * 100))
        Dest = OutDest;
    PathFindInfo = RequestFindPath(Start, Dest);
}

void ATwinLinkNavSystem::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    if (PathFindInfo.has_value() && PathFindInfo->PathFindResult.IsSuccessful()) {
        auto bBeforeSuccess = PathFindInfo->IsSuccess();
        PathFindInfo->Update(GetWorld(), DemCollisionChannel, DemCollisionAabb.Min.Z, DemCollisionAabb.Max.Z, 10);
        auto bAfterSuccess = PathFindInfo->IsSuccess();
        if (bBeforeSuccess == false && bAfterSuccess) {
            for (auto Child : PathDrawers) {
                if (Child)
                    Child->DrawPath(PathFindInfo->HeightCheckedPoints);
            }
        }
    }

    // 
    const APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PlayerController)
        return;

    if (PlayerController->WasInputKeyJustPressed(EKeys::LeftMouseButton)) {
        UKismetSystemLibrary::PrintString(this, TEXT("KeyDown"), true, true, FColor::Cyan, 10.0f, TEXT("None"));

        float MouseX, MouseY;
        if (PlayerController->GetMousePosition(MouseX, MouseY)) {
            FVector WorldPosition, WorldDirection;
            if (UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(MouseX, MouseY), WorldPosition, WorldDirection)) {
                // 始点->道モデルのAABBの中心点までの距離 + Boxの対角線の長さがあれば道モデルとの
                const auto ToAabbVec = DemCollisionAabb.GetCenter() - WorldPosition;
                const auto Length = ToAabbVec.Length() + (DemCollisionAabb.Max - DemCollisionAabb.Min).Length();
                const auto Start = WorldPosition + WorldDirection * 100;
                const auto End = WorldPosition + WorldDirection * Length;
                FHitResult HitResult;
                if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, DemCollisionChannel)) {
                    if (const auto Actor = GetNowSelectedPointActor())
                        Actor->SetActorLocation(HitResult.Location);
                }
            }
        }
    }
#ifdef WITH_EDITOR
    DebugDraw();
#endif
}

FVector ATwinLinkNavSystem::GetStartPoint() const {
    if (PathLocatorStartActor)
        return PathLocatorStartActor->GetActorLocation();
    return FVector::Zero();
}

void ATwinLinkNavSystem::SetStartPoint(const FVector& V) {
    if (PathLocatorStartActor)
        PathLocatorStartActor->SetActorLocation(V);
}

FVector ATwinLinkNavSystem::GetDestPoint() const {
    if (PathLocatorDestActor)
        return PathLocatorDestActor->GetActorLocation();
    return FVector::Zero();
}

void ATwinLinkNavSystem::SetDestPoint(const FVector& V) {
    if (PathLocatorDestActor)
        PathLocatorDestActor->SetActorLocation(V);
}


AActor* ATwinLinkNavSystem::GetNowSelectedPointActor() {
    switch (NowSelectedPointType) {
    case NavSystemPathPointType::Dest:
        return PathLocatorDestActor;
    case NavSystemPathPointType::Start:
        return PathLocatorStartActor;
    default:;
    }
    return nullptr;
}

void ATwinLinkNavSystem::DebugDraw() {
#ifdef WITH_EDITOR
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return;
    if (DebugCallFindPath) {
        DebugCallFindPath = false;
        RequestFindPath();
    }

    // 道路モデルのAABB表示
    DrawDebugBox(GetWorld(), DemCollisionAabb.GetCenter(), DemCollisionAabb.GetExtent(), FColor::Red);

#endif
}


void ATwinLinkNavSystem::BeginPlay() {
    Super::BeginPlay();

    PathLocatorStartActor = CreateChildActor(this, PathLocatorStartBp, TEXT("PathLocatorStartActor"));
    PathLocatorDestActor = CreateChildActor(this, PathLocatorDestBp, TEXT("PathLocatorDestActor"));

    CreateChildActor(this, PathDrawerBp, TEXT("PathDrawerActor"));
    ForeachChildActor<AUTwinLinkNavSystemPathDrawer>(this, [&](AUTwinLinkNavSystemPathDrawer* Child) {
        PathDrawers.Add(Child);
        });
    // Input設定を行う
    SetupInput();
}


void ATwinLinkNavSystem::SetupInput() {
    // PlayerControllerを取得する
    APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    // 入力を有効にする
    EnableInput(controller);
}

void ATwinLinkNavSystem::PressedAction() {
    // Pressed
    UKismetSystemLibrary::PrintString(this, TEXT("Pressed"), true, true, FColor::Cyan, 10.0f, TEXT("None"));
}

void ATwinLinkNavSystem::ReleasedAction() {
    // Released
    UKismetSystemLibrary::PrintString(this, TEXT("Released"), true, true, FColor::Cyan, 10.0f, TEXT("None"));
}

void ATwinLinkNavSystem::PressedAxis(const FInputActionValue& Value) {
    // input is a Vector2D
    FVector2D v = Value.Get<FVector2D>();

    // Axis Input Value
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("X:%f Y:%f"), v.X, v.Y), true, true, FColor::Cyan, 10.0f, TEXT("None"));
}