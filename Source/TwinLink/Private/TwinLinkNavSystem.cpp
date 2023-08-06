#include "TwinLinkNavSystem.h"
#include "NavigationSystem.h"

void ATwinLinkNavSystem::RequestFindPath(FVector Start, FVector End) {

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FPathFindingQuery query;
    query.StartLocation = Start;
    query.EndLocation = End;
    auto Result = NavSys->FindPathSync(query, EPathFindingMode::Regular);
}
