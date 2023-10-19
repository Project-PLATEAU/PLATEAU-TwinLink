#pragma once
class APlayerController;
class TWINLINK_API TwinLinkMathEx {
public:
    /*
     * @brief : FVector2D(V.X, V.Y);
     */
    static FVector2D XY(const FVector& V);

    /*
     * @brief : FVector2D(V.X, V.Z);
     */
    static FVector2D XZ(const FVector& V);

    /*
     * @brief : FVector2D(V.Y, V.Z);
     */
    static FVector2D YZ(const FVector& V);

    /*
     * @brief : FVector2D(V.Z, V.Y);
     */
    static FVector2D ZY(const FVector& V);

    /*
     * @brief : FBox2D(XY(V.Min), XY(V.Max));
     */
    static FBox2D XY(const FBox& V) {
        return FBox2D(XY(V.Min), XY(V.Max));
    }

    /*
     * @brief : FBox2D(XZ(V.Min), XZ(V.Max));
     */
    static FBox2D XZ(const FBox& V);

    /*
     * @brief : FBox2D(YZ(V.Min), YZ(V.Max));
     */
    static FBox2D YZ(const FBox& V);

    /*
     * @brief : FBox2D(ZY(V.Min), ZY(V.Max));
     */
    static FBox2D ZY(const FBox& V);

    struct VecI
    {
        int64 X;
        int64 Y;
        int64 Z;
    };

    template<class T, class F>
    static auto Cast(const UE::Math::TVector<T>& V, F&& Func) {
        return VecI{ Func(V.X),  Func(V.Y), Func(V.Z) };
    }

    static VecI FloorToInt64(const FVector& V) {
        return Cast(V, [](double X) { return FMath::FloorToInt64(X); });
    }

    static VecI CeilToInt64(const FVector& V) {
        return Cast(V, [](double X) { return FMath::CeilToInt64(X); });
    }
    /*
     * @bref : (Θ, φ, r)の極座標系を直交座標系(x,y,z)に変換する(角度はRadian)
     */
    static FVector PolarRadian2Cartesian(const FVector& V);

    /*
     *@bref : (Θ, φ, r)の極座標系を直交座標系(x,y,z)に変換する(角度はDegree)
     */
    static FVector PolarDegree2Cartesian(const FVector& V);

    /*
     * @brief : LocationからLookAtを見るときの回転行列を返す. UpVectorは基準となるカメラの上方向
     */
    static FRotationMatrix CreateLookAtMatrix(const FVector& Location, const FVector& LookAt, const FVector& UpVector = FVector::UpVector);

    /*
    * @brief : 直線A, Bの交点を求める. 二つが平行の場合はfalseを返す
    *        : ADir/BDirが正規化されている前提.
    */
    static bool RayRayIntersection(const FVector2D& AOrigin, const FVector2D& ANormalizedDir, const FVector2D& BOrigin, const FVector2D& BNormalizedDir, FVector2D& Out);
    static bool RayRayIntersectionSafe(const FVector2D& AOrigin, const FVector2D& ADir, const FVector2D& BOrigin, const FVector2D& BDir, FVector2D& Out);

    /*
     * @brief : A,Bのボックスの重なった部分ボックスを返す
     */
    static FBox GetIntersectBox(const FBox& A, const FBox& B);

    /*
     * @brief: 最上位ビットの位置を求める
     */
    static int Nlz(uint64 x) {
        for(auto i = 63; i >= 0; --i)
        {
            if ((x >> i) != 0)
                return i + 1;
        }
        return 0;
      /*  uint64 y;
        int n = 64;
        y = x >> 32; if (y != 0) { n = n - 32; x = y; }
        y = x >> 16; if (y != 0) { n = n - 16; x = y; }
        y = x >> 8; if (y != 0) { n = n - 8; x = y; }
        y = x >> 4; if (y != 0) { n = n - 4; x = y; }
        y = x >> 2; if (y != 0) { n = n - 2; x = y; }
        y = x >> 1; if (y != 0) { return n - 2; }
        return n - x;*/
    }
};
