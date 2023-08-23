#include "TwinLinkNavSystemFindPathInfo.h"
#include "NavigationSystem.h"
#include "Components/InputComponent.h" 

bool TwinLinkNavSystemFindPathInfo::IsRequesting() const {
    if (PathFindResult.Path == nullptr)
        return false;
    // まだ高さ調整位置検索中
    return NowHeightCheckIndex < HeightCheckedPoints.Num();
}

bool TwinLinkNavSystemFindPathInfo::IsValid() const {
    return PathFindResult.Path != nullptr;
}

bool TwinLinkNavSystemFindPathInfo::IsSuccess() const {
    return IsValid() && PathFindResult.IsSuccessful() && IsRequesting() == false;
}

void TwinLinkNavSystemFindPathInfo::Update(const UWorld* World, const ECollisionChannel Channel, const float MinZ, const float MaxZ, const int32_t CheckNum) {
    if (PathFindResult.IsSuccessful() == false)
        return;
    const auto EndI = std::min(NowHeightCheckIndex + CheckNum, HeightCheckedPoints.Num());
    for (; NowHeightCheckIndex < EndI; NowHeightCheckIndex++) {
        auto& Pos = HeightCheckedPoints[NowHeightCheckIndex];
        auto HeightMax = Pos;
        HeightMax.Z = MaxZ + 1;
        auto HeightMin = Pos;
        HeightMin.Z = MinZ - 1;
        FHitResult HeightResult;

        if (World->LineTraceSingleByChannel(HeightResult, HeightMax, HeightMin, Channel))
            Pos.Z = HeightResult.Location.Z + 1;
    }
}

TwinLinkNavSystemFindPathInfo::TwinLinkNavSystemFindPathInfo(FPathFindingResult&& Result)
    : PathFindResult(std::move(Result)) {
}