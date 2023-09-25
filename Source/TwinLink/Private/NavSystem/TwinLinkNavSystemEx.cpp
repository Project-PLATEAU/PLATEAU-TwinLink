#include "NavSystem/TwinLinkNavSystemEx.h"
#include "NavSystem/TwinLinkNavSystemFindPathUiInfo.h"
bool UTwinLinkNavSystemEx::IsValidBuilding(const FTwinLinkNavSystemBuildingInfo& Info) {
    return Info.FacilityInfo != nullptr;
}

FVector UTwinLinkNavSystemEx::GetFirstEntranceOrZero(const FTwinLinkNavSystemBuildingInfo& Info) {
    if (!IsValidBuilding(Info))
        return FVector::Zero();
    FVector Out;
    if (Info.FacilityInfo->TryGetFirstEntrance(Out))
        return Out;
    return FVector::Zero();
}

bool UTwinLinkNavSystemEx::TryGetBuildingInfo(const FTwinLinkNavSystemBuildingInfo& Info, FString& OutBuildingName,
    FString& OutBuildingIconKey, FVector& OutEntrance)
{
    if (!IsValidBuilding(Info))
        return false;
    OutBuildingName = Info.FacilityInfo->GetName();
    OutBuildingIconKey = Info.FacilityInfo->GetImageFileName();
    OutEntrance = GetFirstEntranceOrZero(Info);
    return true;
}
