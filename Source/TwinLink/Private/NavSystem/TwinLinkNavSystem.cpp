#include "NavSystem/TwinLinkNavSystem.h"
#include "NavigationSystem.h"
#include "TwinLinkActorEx.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h" 
#include "NavSystem/TwinLinkNavSystemPathFinder.h"
#include "NavSystem/TwinLinkNavSystemPathFinder.h"

namespace {
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

void ATwinLinkNavSystem::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    if (PathFindInfo.has_value()) {
        const auto bBeforeSuccess = PathFindInfo->IsSuccess();
        PathFindInfo->Update(GetWorld(), DemCollisionChannel, DemCollisionAabb.Min.Z, DemCollisionAabb.Max.Z, 10);
        const auto bAfterSuccess = PathFindInfo->IsSuccess();
        if (bBeforeSuccess == false && bAfterSuccess) {
            for (const auto Child : PathDrawers) {
                if (Child)
                    Child->DrawPath(PathFindInfo->HeightCheckedPoints);
            }
        }
    }

#ifdef WITH_EDITOR
    DebugDraw();
#endif
}

void ATwinLinkNavSystem::DebugDraw() {
#ifdef WITH_EDITOR
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return;

    // 道路モデルのAABB表示
    DrawDebugBox(GetWorld(), DemCollisionAabb.GetCenter(), DemCollisionAabb.GetExtent(), FColor::Red);

    if (DebugCallPathFinding) {
        DebugCallPathFinding = false;
        if (NowPathFinder) {
            TwinLinkNavSystemFindPathInfo Tmp;
            if (NowPathFinder->RequestStartPathFinding(Tmp)) {
                PathFindInfo = Tmp;
            }

        }
    }


#endif
}


void ATwinLinkNavSystem::BeginPlay() {
    Super::BeginPlay();

    TwinLinkActorEx::SpawnChildActor(this, PathDrawerBp, TEXT("PathDrawerActor"));
    ForeachChildActor<AUTwinLinkNavSystemPathDrawer>(this, [&](AUTwinLinkNavSystemPathDrawer* Child) {
        PathDrawers.Add(Child);
        });


    // Input設定を行う
    SetupInput();

    ChangeMode(NavSystemMode::FindPathAnyPoint, true);
}

void ATwinLinkNavSystem::ChangeMode(NavSystemMode Mode, bool bForce) {
    if (NowSelectedMode == Mode && !bForce)
        return;
    if (NowPathFinder)
        NowPathFinder->Destroy();
    if (PathFinderBp.Contains(Mode)) {
        NowPathFinder = TwinLinkActorEx::SpawnChildActor(this, PathFinderBp[Mode], TEXT("PathFinder"));
        NowPathFinder->OnReadyPathFinding.AddUObject(this, &ATwinLinkNavSystem::OnReadyPathFinding);
    }
}

void ATwinLinkNavSystem::OnReadyPathFinding()
{
    if (NowPathFinder) {
        TwinLinkNavSystemFindPathInfo Tmp;
        if (NowPathFinder->RequestStartPathFinding(Tmp)) {
            PathFindInfo = Tmp;
        }
    }
}

void ATwinLinkNavSystem::SetupInput() {
    // PlayerControllerを取得する
    APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    // 入力を有効にする
    EnableInput(controller);
    // マウスカーソルオンにする
    controller->SetShowMouseCursor(true);
}
