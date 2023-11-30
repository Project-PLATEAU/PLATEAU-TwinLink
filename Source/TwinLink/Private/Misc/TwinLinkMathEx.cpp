#include "Misc/TwinLinkMathEx.h"

#include <array>
#include <optional>

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

FBox2D TwinLinkMathEx::XZ(const FBox& V) {
    return FBox2D(XZ(V.Min), XZ(V.Max));
}

FBox2D TwinLinkMathEx::YZ(const FBox& V) {
    return FBox2D(YZ(V.Min), YZ(V.Max));
}

FBox2D TwinLinkMathEx::ZY(const FBox& V) {
    return FBox2D(ZY(V.Min), ZY(V.Max));;
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

FBox TwinLinkMathEx::GetIntersectBox(const FBox& A, const FBox& B) {
    const auto Min = FVector::Max(A.Min, B.Min);
    const auto Max = FVector::Min(A.Max, B.Max);
    // Min が Max より大きい場合、重なる部分は存在しない
    if (Min.X > Max.X || Min.Y > Max.Y || Min.Z > Max.Z)
        return FBox(FVector::ZeroVector, FVector::ZeroVector);

    return FBox(Min, Max);
}

FBox2D TwinLinkMathEx::GetIntersectBoxXY(const FBox& A, const FBox& B) {
    const auto Min = XY(FVector::Max(A.Min, B.Min));
    const auto Max = XY(FVector::Min(A.Max, B.Max));
    // Min が Max より大きい場合、重なる部分は存在しない
    if (Min.X > Max.X || Min.Y > Max.Y)
        return FBox2D(FVector2D::ZeroVector, FVector2D::ZeroVector);

    return FBox2D(Min, Max);
}

FBox2D TwinLinkMathEx::GetIntersectBox2D(const FBox2D& A, const FBox2D& B) {
    const auto Min = FVector2D::Max(A.Min, B.Min);
    const auto Max = FVector2D::Min(A.Max, B.Max);
    // Min が Max より大きい場合、重なる部分は存在しない
    if (Min.X > Max.X || Min.Y > Max.Y)
        return FBox2D(FVector2D::ZeroVector, FVector2D::ZeroVector);

    return FBox2D(Min, Max);
}

double TwinLinkMathEx::GetAreaXY(const FBox& A) {
    return (A.Max.X - A.Min.X) * (A.Max.Y - A.Min.Y);
}

FVector2D TwinLinkMathEx::GetClosestPointOnEdge(const FBox2D& Self, const FVector2D& P) {
    if (Self.IsInside(P) == false)
        return Self.GetClosestPointTo(P);

    const auto Max = Self.Max - P;
    const auto Min = P - Self.Min;
    // 辺との差
    const double Diffs[] = { Min.X, Max.X, Min.Y, Max.Y };
    const double Values[] = { Self.Min.X, Self.Max.X, Self.Min.Y, Self.Max.Y };
    auto Index = 0;
    for (auto I = 1; I < 4; ++I)
        if (Diffs[I] < Diffs[Index])
            Index = I;
    // Xを移動させるのが一番近い場合
    if (Index <= 1)
        return FVector2D(Values[Index], P.Y);
    // Yを移動させるのが違い場合
    return FVector2D(P.X, Values[Index]);
}

FBox2D TwinLinkMathEx::MoveBox2DToNonOverlapPoint(const FBox2D& P, const FBox2D& A, const FBox2D& B,
    const FBox2D& ScreenRange)
{
    if (!A.Intersect(P) && !B.Intersect(P))
        return P;

    // AとPが重ならない境界Box
    const auto BorderA = A.ExpandBy(P.GetExtent());
    const auto BorderB = B.ExpandBy(P.GetExtent());

    // 両方の境界が重ならないならどっちかとしか重なっていない. なおかつ,それとの境界チェックだけすればよい
    if (BorderA.Intersect(BorderB) == false) {
        if (A.Intersect(P))
            return P.MoveTo(GetClosestPointOnEdge(BorderA, P.GetCenter()));
        return P.MoveTo(GetClosestPointOnEdge(BorderB, P.GetCenter()));
    }

    struct Segment {
        FVector2D Start;
        FVector2D End;
    };
    // 各矩形の辺を重なっていない部分だけまとめて線分にする
    TArray<Segment> Edges;
    auto AddEdge = [&](const FVector2D& Start, const FVector2D& End) {
        if (ScreenRange.IsInsideOrOn(Start) && ScreenRange.IsInsideOrOn(End))
            Edges.Add({ Start, End });
    };

    auto AddSegment = [&](const FBox2D& Self, const FBox2D& Other) {
        auto CheckY = [&](float X) {
            if (Self.Min.Y < Other.Min.Y)
                AddEdge(FVector2D(X, Self.Min.Y), FVector2D(X, Other.Min.Y));
            if (Self.Max.Y > Other.Max.Y)
                AddEdge(FVector2D(X, Other.Max.Y), FVector2D(X, Self.Max.Y));
        };
        // Left辺(Min.X)
        // 重なっているときは共通部分もアウトラインなので<=で比較
        if (Self.Min.X <= Other.Min.X) {
            AddEdge( FVector2D(Self.Min.X, Self.Min.Y), FVector2D(Self.Min.X, Self.Max.Y));
        }
        // Self/Otherは重なっている前提なので
        // Self.Min.X > Other.Max.Xの時は考えない
        else {
            CheckY(Self.Min.X);
        }

        // Right(Max.X)
        if (Self.Max.X >= Other.Max.X) {
            AddEdge( FVector2D(Self.Max.X, Self.Min.Y), FVector2D(Self.Max.X, Self.Max.Y));
        }
        else {
            CheckY(Self.Max.Y);
        }

        auto CheckX = [&](float Y) {
            if (Self.Min.X < Other.Min.X)
                AddEdge( FVector2D(Self.Min.X, Y), FVector2D(Other.Min.X, Y));
            if (Self.Max.X > Other.Max.X)
                AddEdge( FVector2D(Other.Max.X, Y), FVector2D(Self.Max.X, Y));
        };
        if (Self.Min.Y <= Other.Min.Y) {
            AddEdge( FVector2D(Self.Min.X, Self.Min.Y), FVector2D(Self.Max.X, Self.Min.Y));
        }
        else {
            CheckX(Self.Min.Y);
        }

        if (Self.Max.Y >= Other.Max.Y) {
            AddEdge( FVector2D(Self.Min.X, Self.Max.Y), FVector2D(Self.Max.X, Self.Max.Y));
        }
        else {
            CheckX(Self.Max.Y);
        }
    };
    AddSegment(BorderA, BorderB);
    AddSegment(BorderB, BorderA);
    if (Edges.IsEmpty())
        return P;

    std::optional< std::tuple<FVector2D, float>> Ans;
    for (auto I = 0; I < Edges.Num(); ++I) {
        auto ClosestPos = FMath::ClosestPointOnSegment2D(P.GetCenter(), Edges[I].Start, Edges[I].End);
        auto SqrLen = (ClosestPos - P.GetCenter()).SquaredLength();
        if (Ans.has_value() == false || SqrLen < std::get<1>(*Ans))
            Ans = std::make_tuple(ClosestPos, SqrLen);
    }
    return P.MoveTo(std::get<0>(*Ans));
}
