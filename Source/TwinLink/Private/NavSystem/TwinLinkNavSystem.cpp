#include "NavSystem/TwinLinkNavSystem.h"

#include <array>
#include <numeric>

#include "NavigationSystem.h"
#include "PLATEAUInstancedCityModel.h"
#include "TwinLinkActorEx.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h" 
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
    // 0, 1, 2, 3... -> 0, +1, -1, +2, -2と変換する
    int AlterSignSequence(int Index) {
        const auto Sign = ((Index & 1) << 1) - 1;
        const auto Offset = (Index + 1) >> 1;
        return Sign * Offset;
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
            OnReadyFindPathInfo(*PathFindInfo);
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
            FTwinLinkNavSystemFindPathInfo Tmp;
            if (NowPathFinder->RequestStartPathFinding(Tmp)) {
                PathFindInfo = Tmp;
            }

        }
    }

    // NavSys->NavDataSet[0]->
#endif
}


void ATwinLinkNavSystem::BeginPlay() {
    Super::BeginPlay();
}

void ATwinLinkNavSystem::ChangeMode(NavSystemMode Mode, bool bForce) {
    if (NowSelectedMode == Mode && !bForce)
        return;

    // 既存のDrawerクラスなどを削除する
    Clear();

    NowSelectedMode = Mode;
    if (NavSystemModeT(NowSelectedMode).IsValid() == false)
        return;

    if (!RuntimeParam) {
        UKismetSystemLibrary::PrintString(this, "NavSystem RuntimeParam != nullptr");
        return;
    }
    // パス描画クラスを再生成する
    TwinLinkActorEx::SpawnChildActor(this, RuntimeParam->PathDrawerBp, TEXT("PathDrawerActor"));
    ForeachChildActor<AUTwinLinkNavSystemPathDrawer>(this, [&](AUTwinLinkNavSystemPathDrawer* Child) {
        PathDrawers.Add(Child);
        });

    if (RuntimeParam->PathFinderBp.Contains(Mode)) {
        FString Name = TEXT("PathFinder");
        Name.AppendInt(static_cast<int>(Mode));
        NowPathFinder = TwinLinkActorEx::SpawnChildActor(this, RuntimeParam->PathFinderBp[Mode], TCHAR_TO_WCHAR(*Name));
        NowPathFinder->OnReadyPathFinding.AddUObject(this, &ATwinLinkNavSystem::OnReadyPathFinding);
    }
}

bool ATwinLinkNavSystem::TryGetReadyFindPathInfo(FTwinLinkNavSystemFindPathInfo& Out) const {
    if (PathFindInfo.has_value()) {
        Out = *PathFindInfo;
        return true;
    }
    return false;
}

const UTwinLinkNavSystemParam* ATwinLinkNavSystem::GetRuntimeParam() const {
    return RuntimeParam;
}

FTwinLinkNavSystemFindPathUiInfo ATwinLinkNavSystem::GetDrawMoveTimeUiInfo(const FBox2D& ScreenRange) const {
    if (PathFindInfo.has_value() == false || PathFindInfo->IsSuccess() == false)
        return FTwinLinkNavSystemFindPathUiInfo();

    const APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    auto& HeightCheckedPoints = PathFindInfo->HeightCheckedPoints;

    // 総距離
    auto Length = 0.f;
    for (auto I = 0; I < HeightCheckedPoints.Num() - 1; ++I) {
        Length += (HeightCheckedPoints[I + 1] - HeightCheckedPoints[I]).Length();
    }

    const auto Meter = Length / RuntimeParam->WorldUnitMeter;
    const auto Sec = Meter * 3.6f / RuntimeParam->WalkSpeedKmPerH;

    const auto Center = FMath::Max(0, HeightCheckedPoints.Num() - 1) >> 1;
    for (auto I = 0; I < HeightCheckedPoints.Num(); ++I) {
        const auto Index = Center + AlterSignSequence(I);
        FVector2D Tmp;
        if (UGameplayStatics::ProjectWorldToScreen(Controller, HeightCheckedPoints[Index], Tmp)) {
            if (ScreenRange.IsInsideOrOn(Tmp)) {
                return FTwinLinkNavSystemFindPathUiInfo(Meter, Sec, Tmp);
            }
        }
    }
    return FTwinLinkNavSystemFindPathUiInfo();
}

ATwinLinkNavSystemPathFinder* ATwinLinkNavSystem::GetNowPathFinder() {
    return NowPathFinder;
}

TArray<FTwinLinkNavSystemBuildingInfo> ATwinLinkNavSystem::GetBuildingInfo() const {

    TArray<FTwinLinkNavSystemBuildingInfo> Ret;
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return Ret;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APLATEAUInstancedCityModel::StaticClass(), AllActors);

    for (auto Actor : AllActors) {
        const auto RootComp = Actor->GetRootComponent();
        if (!RootComp)
            continue;
        for (auto Child : RootComp->GetAttachChildren()) {
            FString MeshName;
            Child->GetName(MeshName);
            // とりあえずBldg以下をすべて返す
            if (!MeshName.Contains("bldg"))
                continue;

            for (auto Lod : Child->GetAttachChildren()) {
                FString LodName;
                Lod->GetName(LodName);
                if (!LodName.StartsWith("LOD2"))
                    continue;

                TArray<USceneComponent*> ChildComponents;
                Lod->GetChildrenComponents(true, ChildComponents);
                for (const auto ChildComp : ChildComponents) {
                    const auto StaMeshComp = Cast<UStaticMeshComponent>(ChildComp);
                    if (!StaMeshComp)
                        continue;
                    StaMeshComp->SetCanEverAffectNavigation(true);
                    const auto StaMesh = StaMeshComp->GetStaticMesh();
                    if (!StaMesh)
                        continue;
                    FString Name;
                    Lod->GetName(Name);
                    auto Bounds = StaMesh->GetBounds();
                    auto Bb = Bounds.GetBox();
                    // ナビメッシュの範囲内かどうか
                    auto Pos = Bb.GetCenter();
                    Pos.Z = 0.f;
                    auto Size = Bb.GetSize();
                    FNavLocation OutPos;
                    if (NavSys->ProjectPointToNavigation(Pos, OutPos, Size + FVector::One() * 1000)) {
                        FTwinLinkNavSystemBuildingInfo Elem;
                        Elem.Name = MeshName;
                        Elem.NavMeshEntranceLocation = OutPos;
                        Elem.NavMeshEntranceLocation.Z = Bb.GetCenter().Z;
                        Ret.Add(Elem);
                    }
                }
            }
        }
    }
    return Ret;
}

void ATwinLinkNavSystem::Clear() {
    if (NowPathFinder) {
        NowPathFinder->Clear();
        NowPathFinder->Destroy();
    }
    NowPathFinder = nullptr;
    // 既存のパス描画クラスを削除
    for (const auto Drawer : PathDrawers)
        Drawer->Destroy();
    PathDrawers.RemoveAll([](auto& a) { return true; });
    PathFindInfo = std::nullopt;
    NowSelectedMode = NavSystemMode::Undefined;
}

void ATwinLinkNavSystem::OnReadyPathFinding() {
    if (NowPathFinder) {
        FTwinLinkNavSystemFindPathInfo Tmp;
        if (NowPathFinder->RequestStartPathFinding(Tmp)) {
            PathFindInfo = Tmp;
        }
    }
}
