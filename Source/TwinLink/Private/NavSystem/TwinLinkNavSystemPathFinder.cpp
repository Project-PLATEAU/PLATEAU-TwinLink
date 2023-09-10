// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "NavSystem/TwinLinkNavSystemPathFinder.h"

#include <optional>

#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include <NavSystem/TwinLinkNavSystemPathLocator.h>

#include "TwinLinkActorEx.h"
#include "NavSystem/TwinLinkNavSystem.h"
namespace {

    std::optional<FVector2D> GetMousePosition(const APlayerController* PlayerController) {
        if (!PlayerController)
            return std::nullopt;
        float MouseX, MouseY;
        if (PlayerController->GetMousePosition(MouseX, MouseY)) {
            return FVector2D(MouseX, MouseY);
        }
        return std::nullopt;
    }

    // マウスカーソル位置におけるDemCollisionを包括するような線分を取得
    // Min/Maxが始点/終点を表す
    std::optional<FBox> GetMouseCursorWorldVector(const APlayerController* PlayerController, const FVector2D& MousePos, const FBox& RangeAabb) {
        if (!PlayerController)
            return std::nullopt;
        FVector WorldPosition, WorldDirection;
        if (UGameplayStatics::DeprojectScreenToWorld(PlayerController, MousePos, WorldPosition, WorldDirection)) {
            // 始点->道モデルのAABBの中心点までの距離 + Boxの対角線の長さがあれば道モデルとの
            const auto ToAabbVec = RangeAabb.GetCenter() - WorldPosition;
            const auto Length = ToAabbVec.Length() + (RangeAabb.Max - RangeAabb.Min).Length();
            return FBox(WorldPosition + WorldDirection * 100, WorldPosition + WorldDirection * Length);
        }
        return std::nullopt;
    }

    bool CreateFindPathRequest(const UObject* Self, const FVector& Start, const FVector& End, FPathFindingQuery& OutRequest) {
        const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Self->GetWorld());
        if (!NavSys)
            return false;
        OutRequest = FPathFindingQuery();
        OutRequest.StartLocation = Start;
        OutRequest.EndLocation = End;
        // #NOTE : ナビメッシュは車と人で区別しないということなのでエージェントは設定しない
        //query.NavAgentProperties.AgentHeight = 100;
        //query.NavAgentProperties.AgentRadius = 10;
        OutRequest.NavData = NavSys->GetDefaultNavDataInstance();
        OutRequest.Owner = Self;
        return true;
    }
}
// Sets default values
ATwinLinkNavSystemPathFinder::ATwinLinkNavSystemPathFinder() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

bool ATwinLinkNavSystemPathFinder::RequestStartPathFinding(FTwinLinkNavSystemFindPathInfo& Out) {
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return false;
    // まだ両地点生成済みじゃない場合は無視する
    if (IsReadyPathFinding() == false)
        return false;
    auto Start = *GetPathLocation(NavSystemPathPointType::Start);
    auto Dest = *GetPathLocation(NavSystemPathPointType::Dest);
    Start.Z = Dest.Z = 0;
    Out = RequestPathFinding(Start, Dest);
    return true;
}

// Called when the game starts or when spawned
void ATwinLinkNavSystemPathFinder::BeginPlay() {
    Super::BeginPlay();
}

void ATwinLinkNavSystemPathFinder::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

}

const ATwinLinkNavSystem* ATwinLinkNavSystemPathFinder::GetTwinLinkNavSystem() const {
    return TwinLinkActorEx::FindActorInOwner<ATwinLinkNavSystem>(this);
}

FTwinLinkNavSystemFindPathInfo ATwinLinkNavSystemPathFinder::RequestPathFinding(const FVector& Start,
    const FVector& End) const {
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FPathFindingQuery Query;
    CreateFindPathRequest(this, Start, End, Query);
    FTwinLinkNavSystemFindPathInfo Ret = FTwinLinkNavSystemFindPathInfo(NavSys->FindPathSync(Query, EPathFindingMode::Regular));
    const auto TwinLinkNavSystem = GetTwinLinkNavSystem();
    const auto DemCollisionAabb = TwinLinkNavSystem->GetDemCollisionAabb();
    const auto DemCollisionChannel = TwinLinkNavSystem->GetDemCollisionChannel();
    const auto HeightCheckInterval = TwinLinkNavSystem->GetPathFindingHeightCheckInterval();
    if (Ret.PathFindResult.IsSuccessful()) {
        auto GetHeightCheckedPos = [this, &DemCollisionAabb, &DemCollisionChannel](FVector Pos) {
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
                auto CheckNum = static_cast<int32>((Length - ModLength) / HeightCheckInterval);
                for (auto I = 1; I <= CheckNum; ++I) {
                    auto P = GetHeightCheckedPos(LastPos + Dir * HeightCheckInterval);
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

ATwinLinkNavSystemPathFinderAnyLocation::ATwinLinkNavSystemPathFinderAnyLocation() {
}

void ATwinLinkNavSystemPathFinderAnyLocation::BeginPlay() {
    Super::BeginPlay();
}

// Called every frame
void ATwinLinkNavSystemPathFinderAnyLocation::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    const auto TwinLinkNavSystem = GetTwinLinkNavSystem();
    const auto DemCollisionAabb = TwinLinkNavSystem->GetDemCollisionAabb();

    // 
    const APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PlayerController)
        return;
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return;
    // 押したとき
    auto MousePos = ::GetMousePosition(PlayerController);
    if (MousePos.has_value() == false)
        return;
    if (PlayerController->WasInputKeyJustPressed(EKeys::LeftMouseButton)) {
        UKismetSystemLibrary::PrintString(this, TEXT("KeyDown"), true, true, FColor::Cyan, 10.0f, TEXT("None"));

        auto Ray = ::GetMouseCursorWorldVector(PlayerController, *MousePos, DemCollisionAabb);
        if (Ray.has_value()) {
            TArray<FHitResult> HitResults;
            FCollisionQueryParams Params;
            if (GetWorld()->LineTraceMultiByChannel(HitResults, Ray->Min, Ray->Max, ECC_WorldStatic)) {
                auto PointType = GetNextCreatePointType();
                FHitResult* BlockHitResult = nullptr;
                NowSelectedPathLocatorActor = nullptr;
                for (auto& HitResult : HitResults) {

                    UKismetSystemLibrary::PrintString(this, HitResult.GetActor()->GetActorLabel(), true, true, FColor::Cyan, 10.0f, TEXT("None"));
                    if (HitResult.IsValidBlockingHit())
                        BlockHitResult = &HitResult;
                    // Start/Destが重なっていた場合最初にヒットしたほうを選択する
                    NowSelectedPathLocatorActor = Cast<ATwinLinkNavSystemPathLocator>(HitResult.GetActor());
                    if (NowSelectedPathLocatorActor) {
                        NowSelectedPathLocatorActorScreenOffset = FVector2D::Zero();
                        FVector2D ScreenPos = FVector2D::Zero();
                        if (UGameplayStatics::ProjectWorldToScreen(PlayerController, NowSelectedPathLocatorActor->GetActorLocation(), ScreenPos))
                            NowSelectedPathLocatorActorScreenOffset = ScreenPos - *MousePos;
                        break;
                    }
                }

                if (BlockHitResult != nullptr && NowSelectedPathLocatorActor == nullptr && PointType.IsValid()) {
                    FString Name = TEXT("PathLocator");
                    Name.AppendInt(PointType.GetValue());
                    NowSelectedPathLocatorActor = TwinLinkActorEx::SpawnChildActor(this, PathLocatorBps[PointType.GetEnumValue()], ToCStr(Name));
                    NowSelectedPathLocatorActor->SetActorLocation(BlockHitResult->Location);
                    PathLocatorActors.Add(PointType.GetEnumValue(), NowSelectedPathLocatorActor);
                    SetNowSelectedPointType(static_cast<NavSystemPathPointType>(PointType.GetValue() + 1));
                }
                if (NowSelectedPathLocatorActor) {
                    NowSelectedPathLocatorActor->Select();
                }
            }
        }
    }
    // 離した時
    else if (PlayerController->WasInputKeyJustReleased(EKeys::LeftMouseButton)) {
        UKismetSystemLibrary::PrintString(this, TEXT("KeyUp"), true, true, FColor::Cyan, 1.0f, TEXT("None"));
        if (NowSelectedPathLocatorActor) {
            NowSelectedPathLocatorActor->UnSelect();

            if (IsReadyPathFinding())
                OnReadyPathFinding.Broadcast();
        }
        NowSelectedPathLocatorActor = nullptr;
    }
    else if (PlayerController->IsInputKeyDown(EKeys::LeftMouseButton)) {

        UKismetSystemLibrary::PrintString(this, TEXT("Drag"), true, true, FColor::Cyan, 0.f, TEXT("None"));
        if (NowSelectedPathLocatorActor) {
            UKismetSystemLibrary::PrintString(this, TEXT("MoveActor"), true, true, FColor::Cyan, 0.f, TEXT("None"));
            const auto ScreenPos = *MousePos + NowSelectedPathLocatorActorScreenOffset;
            const auto Ray = ::GetMouseCursorWorldVector(PlayerController, ScreenPos, DemCollisionAabb);
            if (Ray.has_value()) {
                TArray<FHitResult> HitResults;
                if (GetWorld()->LineTraceMultiByChannel(HitResults, Ray->Min, Ray->Max, ECollisionChannel::ECC_WorldStatic)) {
                    for (auto& HitResult : HitResults) {
                        UKismetSystemLibrary::PrintString(this, TEXT("MoveHit"), true, true, FColor::Cyan, 0.f, TEXT("None"));
                        UKismetSystemLibrary::PrintString(this, HitResult.GetActor()->GetActorLabel(), true, true, FColor::Cyan, 1.0f, TEXT("None"));
                        if (HitResult.IsValidBlockingHit() == false)
                            continue;

                        NowSelectedPathLocatorActor->UpdateLocation(NavSys, HitResult);
                        break;
                    }
                }
            }
        }
    }
}

bool ATwinLinkNavSystemPathFinderAnyLocation::IsReadyPathFinding() const {
    if (GetNextCreatePointType().IsValid())
        return false;

    return GetPathLocation(NavSystemPathPointType::Start).has_value() && GetPathLocation(NavSystemPathPointType::Dest).has_value();
}

std::optional<FVector> ATwinLinkNavSystemPathFinderAnyLocation::GetPathLocation(NavSystemPathPointTypeT Type) const {
    if (PathLocatorActors.Contains(Type.GetEnumValue())) {
        if (const auto Actor = PathLocatorActors[Type.GetEnumValue()])
            return Actor->GetLastValidLocation();
    }
    return std::nullopt;
}

ATwinLinkNavSystemPathFinderListSelect::ATwinLinkNavSystemPathFinderListSelect()
{
}

void ATwinLinkNavSystemPathFinderListSelect::BeginPlay()
{
    Super::BeginPlay();
}

void ATwinLinkNavSystemPathFinderListSelect::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool ATwinLinkNavSystemPathFinderListSelect::IsReadyPathFinding() const
{
    return Super::IsReadyPathFinding();
}

