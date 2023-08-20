#include "TwinLinkNavSystem.h"
#include "NavigationSystem.h"

ATwinLinkNavSystem::ATwinLinkNavSystem() {
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

FPathFindingResult ATwinLinkNavSystem::RequestFindPath(const FVector& Start, const FVector& End) {

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FPathFindingQuery Query;
    CreateFindPathRequest(Start, End, Query);
    return NavSys->FindPathSync(Query, EPathFindingMode::Regular);
}

bool ATwinLinkNavSystem::CreateFindPathRequest(const FVector& Start, const FVector& End, FPathFindingQuery& OutRequest) {
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

void ATwinLinkNavSystem::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
#ifdef WITH_EDITOR
    DebugDraw();
#endif
}

void ATwinLinkNavSystem::DebugDraw() {
#ifdef WITH_EDITOR
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return;
    if (!DebugCallFindPath)
        return;

    // 道路モデルのAABB表示
    DrawDebugBox(GetWorld(), DemCollisionAabb.GetCenter(), DemCollisionAabb.GetExtent(), FColor::Red);

    using LocatorType = AActor;
    auto CreateLocatorOrSkip = [self = this](FWeakObjectPtr& Out, const TCHAR* Name)-> LocatorType* {
        if (Out.IsValid() == false) {
            FActorSpawnParameters Params;
            Params.Owner = self;

            Params.Name = FName(Name);
            const auto Ret = self->GetWorld()->SpawnActor<LocatorType>(Params);
            if (!Ret)
                return Ret;
            Ret->SetActorLabel(FString(Name));
            // エディタ上で移動できるようにRootComponentつけておく
            if (const auto Root = NewObject<USceneComponent>(Ret)) {
                Root->RegisterComponent();
                Ret->SetRootComponent(Root);
            }
            Ret->AttachToActor(self, FAttachmentTransformRules::KeepWorldTransform, Name);
            Out = Ret;
        }
        return Cast<LocatorType>(Out.Get());
    };
    const auto StartLocator = CreateLocatorOrSkip(DebugFindPathStart, TEXT("DebugPathFindStart"));
    const auto EndLocator = CreateLocatorOrSkip(DebugFindPathEnd, TEXT("DebugPathFindEnd"));
    if (StartLocator && EndLocator) {
        //FPathFindingQuery Query;
        //CreateFindPathRequest(StartLocator->GetActorLocation(), EndLocator->GetActorLocation(), Query);
        //DebugPathFindDelegate.BindLambda([this](unsigned, ENavigationQueryResult::Type Type, TSharedPtr<FNavigationPath> Path)
        //    {
        //        
        //    });
        //NavSys->FindPathAsync(Query.NavAgentProperties, Query, DebugPathFindDelegate);
        const auto PathFindResult = RequestFindPath(StartLocator->GetActorLocation(), EndLocator->GetActorLocation());

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
        if (PathFindResult.IsSuccessful()) {
            auto& points = PathFindResult.Path->GetPathPoints();
            FVector LastPos = FVector::Zero();
            for (auto i = 0; i < points.Num(); ++i) {
                auto& p = points[i];
                auto Pos = GetHeightCheckedPos(p.Location);
                DrawDebugSphere(GetWorld(), Pos, 5, 10, FColor::Red, false, 3);
                if (i > 0) {
                    auto Dir = (Pos - LastPos);                    
                    auto Length = Dir.Length();
                    Dir.Normalize();
                    auto CheckNum = static_cast<int32>(Length / DebugFindPathHeightCheckInterval);
                    for(auto I = 1; I <= CheckNum; ++I)
                    {
                        auto P = GetHeightCheckedPos(LastPos + Dir * DebugFindPathHeightCheckInterval);
                        DrawDebugLine(GetWorld(), LastPos, P, FColor::Blue);
                        LastPos = P;
                    }
                    DrawDebugLine(GetWorld(), LastPos, Pos, FColor::Blue);
                }
                LastPos = Pos;
            }
        }
    }
#endif
}
