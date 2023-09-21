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

FRotationMatrix TwinLinkMathEx::CreateLookAtMatrix(const FVector& Location, const FVector& LookAt, const FVector& UpVector)
{
    const auto Dir = (LookAt - Location).GetSafeNormal(1e-8f,FVector::ForwardVector);
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
/*
bool TwinLinkMathEx::CalcCameraLocationToFitAllPoint(const APlayerController* PlayerController, const FBox2D& ScreenRange,
    const TArray<FVector>& Points, FVector& OutCameraLocation) {
    if (Points.Num() <= 1)
        return false;

    auto LP = PlayerController->GetLocalPlayer();
    FSceneViewProjectionData ProjectionData;
    if (LP->GetProjectionData(LP->ViewportClient->Viewport, ProjectionData) == false)
        return false;
    // View行列
    const auto ViewMatrix = FTranslationMatrix(-ProjectionData.ViewOrigin) * ProjectionData.ViewRotationMatrix;
    FVector CAxisX, CAxisY, CAxisZ;
    ProjectionData.ViewRotationMatrix.GetUnitAxes(CAxisX, CAxisY, CAxisZ);

    auto ToLocal = [&](FVector W) {
        W = W - ProjectionData.ViewOrigin;
        return FVector(CAxisX.Dot(W), CAxisY.Dot(W), CAxisZ.Dot(W));
    };
    // Left,Right,Bottom,Top,Nearの順
    std::array<FVector, 5> WPos;
    std::array<FVector, 5> LPos;
    WPos.fill(Points[0]);
    LPos.fill(ToLocal(Points[0]));
    for (auto i = 1; i < Points.Num(); ++i) {
        const auto W = Points[i];
        const auto L = ToLocal(W);
        auto Set = [&](int I) {
            LPos[I] = L;
            WPos[I] = W;
        };
        if (L.X < LPos[0].X)
            Set(0);
        if (L.X > LPos[1].X)
            Set(1);
        if (L.Z < LPos[2].Z)
            Set(2);
        if (L.Z > LPos[3].Z)
            Set(3);
        if (L.Y < LPos[4].Y)
            Set(4);
    }

    const float AspectRatioX = 1.f * ProjectionData.GetViewRect().Width() / ProjectionData.GetViewRect().Height();
    const float NearClip = ProjectionData.ProjectionMatrix.M[3][2] / (ProjectionData.ProjectionMatrix.M[2][2] - 1.0f);

    const auto TanFovX = 1.f / ProjectionData.ProjectionMatrix.M[0][0];// FMath::Tan(FovRad * 0.5f);
    const auto TanFovZ = TanFovX / AspectRatioX;
    const auto MinXDir = FVector(TanFovX * ScreenRange.Min.X, 1.f, 0.f).GetUnsafeNormal();
    const auto MaxXDir = FVector(TanFovX * ScreenRange.Max.X, 1.f, 0.f).GetUnsafeNormal();
    const auto MinZDir = FVector(0.f, 1.f, TanFovZ * ScreenRange.Min.Y).GetUnsafeNormal();
    const auto MaxZDir = FVector(0.f, 1.f, TanFovZ * ScreenRange.Max.Y).GetUnsafeNormal();

    FVector2D OutX;
    if (RayRayIntersection(XY(LPos[0]), XY(MinXDir), XY(LPos[1]), XY(MaxXDir), OutX) == false)
        return false;

    FVector2D OutZ;
    if (RayRayIntersection(YZ(LPos[2]), YZ(MinZDir), YZ(LPos[3]), YZ(MaxZDir), OutZ) == false)
        return false;

    FVector LCPos = FVector::Zero();
    LCPos.X = OutX.X;
    LCPos.Z = OutZ.Y;
    LCPos.Y = FMath::Max(OutX.Y, OutZ.X);// FMath::Min(OutX.Y, FMath::Min(OutZ.X, LPos[4].Y - NearClip));
    OutCameraLocation = (LCPos.X * CAxisX + LCPos.Y * CAxisY + LCPos.Z * CAxisZ);// +ProjectionData.ViewOrigin;
    {
        auto CopyProj = ProjectionData;
        CopyProj.ViewOrigin = OutCameraLocation;
        auto ViewProjectionMatrix = CopyProj.ComputeViewProjectionMatrix();
        for (auto I = 0; I < WPos.size(); ++I) {
            auto& W = WPos[I];
            FVector2D ScreenPosition;
            bool bResult = FSceneView::ProjectWorldToScreen(W, ProjectionData.GetViewRect(), ViewProjectionMatrix, ScreenPosition);
            if (bResult) {
                FString F = FString::Printf(TEXT("[%d] %d,%d"), I, (int)ScreenPosition.X, (int)ScreenPosition.Y);
                UKismetSystemLibrary::PrintString(PlayerController, F);
            }
        }
    }
    return true;
}

bool TwinLinkMathEx::CalcCameraLocationToFitTwoPoint(const APlayerController* PlayerController,
    const FBox2D& ScreenRange, const FVector& PA, const FVector& PB, FVector& OutCameraLocation) {
    //auto LP = PlayerController->GetLocalPlayer();
    //FSceneViewProjectionData ProjectionData;
    //if (LP->GetProjectionData(LP->ViewportClient->Viewport, ProjectionData) == false)
    //    return false;
    //// View行列
    //const auto ViewMatrix = FTranslationMatrix(-ProjectionData.ViewOrigin) * ProjectionData.ViewRotationMatrix;
    //FVector CAxisX, CAxisY, CAxisZ;
    //ProjectionData.ViewRotationMatrix.GetUnitAxes(CAxisX, CAxisY, CAxisZ);

    //auto Center = (PA + PB) * 0.5f;
    //auto 
    //const float AspectRatioX = 1.f * ProjectionData.GetViewRect().Width() / ProjectionData.GetViewRect().Height();
    //const float NearClip = ProjectionData.ProjectionMatrix.M[3][2] / (ProjectionData.ProjectionMatrix.M[2][2] - 1.0f);

    //const auto TanFovX = 1.f / ProjectionData.ProjectionMatrix.M[0][0];// FMath::Tan(FovRad * 0.5f);
    //const auto TanFovZ = TanFovX / AspectRatioX;
    //const auto MinXDir = FVector(TanFovX * ScreenRange.Min.X, 1.f, 0.f).GetUnsafeNormal();
    //const auto MaxXDir = FVector(TanFovX * ScreenRange.Max.X, 1.f, 0.f).GetUnsafeNormal();
    //const auto MinZDir = FVector(0.f, 1.f, TanFovZ * ScreenRange.Min.Y).GetUnsafeNormal();
    //const auto MaxZDir = FVector(0.f, 1.f, TanFovZ * ScreenRange.Max.Y).GetUnsafeNormal();

    //FVector2D OutX;
    //if (RayRayIntersection(XY(LPos[0]), XY(MinXDir), XY(LPos[1]), XY(MaxXDir), OutX) == false)
    //    return false;

    //FVector2D OutZ;
    //if (RayRayIntersection(YZ(LPos[2]), YZ(MinZDir), YZ(LPos[3]), YZ(MaxZDir), OutZ) == false)
    //    return false;

    //FVector LCPos = FVector::Zero();
    //LCPos.X = OutX.X;
    //LCPos.Z = OutZ.Y;
    //LCPos.Y = FMath::Max(OutX.Y, OutZ.X);// FMath::Min(OutX.Y, FMath::Min(OutZ.X, LPos[4].Y - NearClip));
    //OutCameraLocation = (LCPos.X * CAxisX + LCPos.Y * CAxisY + LCPos.Z * CAxisZ);// +ProjectionData.ViewOrigin;
    //{
    //    auto CopyProj = ProjectionData;
    //    CopyProj.ViewOrigin = OutCameraLocation;
    //    auto ViewProjectionMatrix = CopyProj.ComputeViewProjectionMatrix();
    //    for (auto I = 0; I < WPos.size(); ++I) {
    //        auto& W = WPos[I];
    //        FVector2D ScreenPosition;
    //        bool bResult = FSceneView::ProjectWorldToScreen(W, ProjectionData.GetViewRect(), ViewProjectionMatrix, ScreenPosition);
    //        if (bResult) {
    //            FString F = FString::Printf(TEXT("[%d] %d,%d"), I, (int)ScreenPosition.X, (int)ScreenPosition.Y);
    //            UKismetSystemLibrary::PrintString(PlayerController, F);
    //        }
    //    }
    //}
    return true;
}
*/
