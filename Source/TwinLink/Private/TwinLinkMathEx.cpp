#include "TwinLinkMathEx.h"

#include <array>

#include "MaterialHLSLTree.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"

FVector2D TwinLinkMathEx::XY(const FVector& V) {
    return FVector2D(V.X, V.Y);
}

FVector2D TwinLinkMathEx::XZ(const FVector& V) {
    return FVector2D(V.X, V.Z);
}

FVector2D TwinLinkMathEx::YZ(const FVector& V) {
    return FVector2D(V.Y, V.Z);
}

FVector2D TwinLinkMathEx::ZY(const FVector& V) {
    return FVector2D(V.Z, V.Y);
}

FVector TwinLinkMathEx::PolarRadian2Cartesian(const FVector& V) {
    double ThetaSi, ThetaCo, PhiSi, PhiCo;
    FMath::SinCos(&ThetaSi, &ThetaCo, V.X);
    FMath::SinCos(&PhiSi, &PhiCo, V.Y);
    return V.Z * FVector(PhiCo * ThetaCo, PhiCo * ThetaSi, PhiSi);
}

FVector TwinLinkMathEx::PolarDegree2Cartesian(const FVector& V) {
    return PolarRadian2Cartesian(FVector(FMath::DegreesToRadians(V.X), FMath::DegreesToRadians(V.Y), V.Z));
}

FRotationMatrix TwinLinkMathEx::CreateLookAtMatrix(const FVector& Location, const FVector& LookAt, const FVector& UpVector) {
    const auto Dir = (LookAt - Location).GetSafeNormal(1e-8f, FVector::ForwardVector);
    return FRotationMatrix::MakeFromXZ(Dir, UpVector).Rotator();
}

bool TwinLinkMathEx::RayRayIntersection(const FVector2D& AOrigin, const FVector2D& ANormalizedDir, const FVector2D& BOrigin,
    const FVector2D& BNormalizedDir, FVector2D& Out) {
    // 平行な場合はfalseを返す
    if (FMath::Abs(ANormalizedDir.Dot(BNormalizedDir)) > (1.f - 1e-6))
        return false;
    const auto V1 = ANormalizedDir;
    const auto A1 = V1.Y;
    const auto B1 = -V1.X;
    const auto C1 = -(AOrigin.X * V1.Y + AOrigin.Y * V1.X);

    const auto V2 = BNormalizedDir;
    const auto A2 = V2.Y;
    const auto B2 = -V2.X;
    const auto C2 = -(BOrigin.X * V2.Y + BOrigin.Y * V2.X);

    const auto D = A1 * B2 - A2 * B1;
    Out = FVector2D(B1 * C2 - B2 * C1, A2 * C1 - A1 * C2) / D;
    return true;
}

bool TwinLinkMathEx::RayRayIntersectionSafe(const FVector2D& AOrigin, const FVector2D& ADir, const FVector2D& BOrigin,
    const FVector2D& BDir, FVector2D& Out) {
    return RayRayIntersection(AOrigin, ADir.GetSafeNormal(), BOrigin, BDir.GetSafeNormal(), Out);
}