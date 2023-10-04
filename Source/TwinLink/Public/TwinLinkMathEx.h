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
};
