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
        if (PathFindResult.IsSuccessful()) {
            auto& points = PathFindResult.Path->GetPathPoints();
            for (auto i = 0; i < points.Num(); ++i) {
                auto& p = points[i];
                auto offset = FVector::UpVector * DebugFindPathUpOffset;

                DrawDebugSphere(GetWorld(), p.Location + offset, 5, 10, FColor::Red, false, 3);
                if (i < points.Num() - 1) {
                    DrawDebugLine(GetWorld(), p.Location + offset, points[i + 1].Location + offset, FColor::Blue);
                }
            }
        }
    }
#endif
}
