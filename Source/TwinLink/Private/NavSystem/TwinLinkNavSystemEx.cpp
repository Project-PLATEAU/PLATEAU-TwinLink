#include "NavSystem/TwinLinkNavSystemEx.h"
#include "NavSystem/TwinLinkNavSystemBuildingInfo.h"
#include "NavSystem/TwinLinkNavSystemFindPathUiInfo.h"
bool UTwinLinkNavSystemEx::IsValidBuilding(const FTwinLinkNavSystemBuildingInfo& Info) {
    return Info.FacilityInfo != nullptr && Info.CityObjectGroup.IsValid();
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
    FString& OutBuildingIconKey, FVector& OutEntrance) {
    if (!IsValidBuilding(Info))
        return false;
    OutBuildingName = Info.FacilityInfo->GetName();
    OutBuildingIconKey = Info.FacilityInfo->GetImageFileName();
    OutEntrance = GetFirstEntranceOrZero(Info);
    return true;
}

FString UTwinLinkNavSystemEx::GetTimeText(const FTwinLinkNavSystemFindPathUiInfo& Info) {
    return FString::Printf(TEXT("%d分"), static_cast<int>(Info.TimeSec / 60));
}

FString UTwinLinkNavSystemEx::GetDistanceTest(const FTwinLinkNavSystemFindPathUiInfo& Info) {
    return FString::Printf(TEXT("%dm"), static_cast<int>(Info.LengthMeter));
}

void UTwinLinkNavSystemEx::GetTimeAndDistanceText(const FTwinLinkNavSystemFindPathUiInfo& Info, FString& OutTimeText,
    FString& OutDistanceText) {
    OutTimeText = GetTimeText(Info);
    OutDistanceText = GetDistanceTest(Info);
}
