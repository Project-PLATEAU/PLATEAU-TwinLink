#include "NavSystem/TwinLinkNavSystemFindPathInfo.h"
#include "NavigationSystem.h"
#include "Components/InputComponent.h" 
#include "Kismet/GameplayStatics.h"

bool FTwinLinkNavSystemFindPathInfo::IsRequesting() const {
    if (PathFindResult.Path == nullptr)
        return false;
    // まだ高さ調整位置検索中
    return NowHeightCheckIndex < HeightCheckedPoints.Num();
}

bool FTwinLinkNavSystemFindPathInfo::IsValid() const {
    return PathFindResult.Path != nullptr;
}

bool FTwinLinkNavSystemFindPathInfo::IsSuccess() const {
    return IsValid() && PathFindResult.IsSuccessful() && IsRequesting() == false;
}

void FTwinLinkNavSystemFindPathInfo::Update(const UWorld* World, const ECollisionChannel Channel, const float MinZ, const float MaxZ, const int32_t CheckNum) {
    if (PathFindResult.IsSuccessful() == false)
        return;
    const auto EndI = std::min(NowHeightCheckIndex + CheckNum, HeightCheckedPoints.Num());
    for (; NowHeightCheckIndex < EndI; NowHeightCheckIndex++) {
        auto& Pos = HeightCheckedPoints[NowHeightCheckIndex];
        // 一応オフセットを入れておく
        auto HeightMax = Pos;
        HeightMax.Z = MaxZ + 1;
        auto HeightMin = Pos;
        HeightMin.Z = MinZ - 1;
        FHitResult HeightResult;
        if (World->LineTraceSingleByChannel(HeightResult, HeightMax, HeightMin, Channel))
            Pos.Z = HeightResult.Location.Z + 1;
    }
}

float FTwinLinkNavSystemFindPathInfo::GetTotalLength() const {
    auto Length = 0.f;
    for (auto I = 0; I < HeightCheckedPoints.Num() - 1; ++I) {
        Length += (HeightCheckedPoints[I + 1] - HeightCheckedPoints[I]).Length();
    }
    return Length;
}

FTwinLinkNavSystemFindPathInfo::FTwinLinkNavSystemFindPathInfo(FPathFindingResult&& Result)
    : PathFindResult(std::move(Result)) {
}
