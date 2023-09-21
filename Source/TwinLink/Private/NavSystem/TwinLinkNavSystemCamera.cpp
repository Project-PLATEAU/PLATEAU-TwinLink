#include "NavSystem/TwinLinkNavSystemCamera.h"

#include "TwinLinkMathEx.h"
#include "NavSystem/TwinLinkNavSystem.h"

namespace {
    bool Lerp(const FVector& From, const FVector& To, float V, FVector& Out, float ToleranceSqrLen = 1e-3f) {
        if ((From - To).SquaredLength() < ToleranceSqrLen) {
            Out = To;
            return true;
        }
        Out = FMath::Lerp(From, To, V);
        return false;
    }
}

ATwinLinkNavSystemCamera::ATwinLinkNavSystemCamera()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

void ATwinLinkNavSystemCamera::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    if (TargetInfo.has_value() && NowInfo.has_value()) {
        FVector NextLocation, NextLookAt;
        auto Coef = DeltaSeconds;
        if(auto NavSystem = ATwinLinkNavSystem::GetInstance(GetWorld()))
        {
            if (auto Param = NavSystem->GetRuntimeParam())
                Coef = Coef * Param->CameraLerpCoef;
        }
        Coef = FMath::Clamp(Coef, 0.f, 1.f);
        const auto bReachLocation = ::Lerp(NowInfo->Location, TargetInfo->Location, Coef, NextLocation);
        const auto bReachLookAt = ::Lerp(NowInfo->LookAt, TargetInfo->LookAt, Coef, NextLookAt);
        SetCameraLookAt(NextLocation, NextLookAt);
        if (bReachLocation && bReachLookAt)
            TargetInfo = std::nullopt;
    }
}

void ATwinLinkNavSystemCamera::SetTargetLookAt(const FVector& Location, const FVector& LookAt, bool bForce) {
    if (NowInfo.has_value() == false)
        NowInfo = Info{ Location, LookAt };
    if (bForce) {
        SetCameraLookAt(Location, LookAt);
        TargetInfo = std::nullopt;
    }
    else {
        TargetInfo = Info{ Location, LookAt };
    }
}

void ATwinLinkNavSystemCamera::SetCameraLookAt(const FVector& Location, const FVector& LookAt) {
    NowInfo = Info{ Location, LookAt };
    SetActorLocation(Location);
    SetActorRotation(TwinLinkMathEx::CreateLookAtMatrix(Location, LookAt).ToQuat());
}
