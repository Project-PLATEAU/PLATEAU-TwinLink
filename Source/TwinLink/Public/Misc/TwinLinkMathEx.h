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

    template<class F, class... Args>
    static auto Cast(F&& Func, Args&&... args) {
        return std::make_tuple(Func(std::forward<Args>(args))...);
    }

    template<class... Args>
    static auto FloorToInt64(Args... args) {
        return Cast([](auto X) { return FMath::FloorToInt64(X); }, std::forward<Args>(args)...);
    }

    template<class... Args>
    static auto CeilToInt64(Args... args) {
        return Cast([](auto X) { return FMath::CeilToInt64(X); }, std::forward<Args>(args)...);
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
     * @brief : A,Bのボックスの重なった部分(XY)ボックスを返す
     */
    static FBox2D GetIntersectBoxXY(const FBox& A, const FBox& B);
    /*
     * @brief : A,Bのボックスの重なった部分ボックスを返す
     */
    static FBox2D GetIntersectBox2D(const FBox2D& A, const FBox2D& B);
    /*
     * @brief : return (Max.X - Min.X) * (Max.Y - Min.Y);
     */
    static double GetAreaXY(const FBox& A);
    /*
     * @brief: 最上位ビットの位置を求める
     */
    static int Nlz(uint64 x) {
        uint64 y;
        int n = 64;
        y = x >> 32; if (y != 0) { n = n - 32; x = y; }
        y = x >> 16; if (y != 0) { n = n - 16; x = y; }
        y = x >> 8; if (y != 0) { n = n - 8; x = y; }
        y = x >> 4; if (y != 0) { n = n - 4; x = y; }
        y = x >> 2; if (y != 0) { n = n - 2; x = y; }
        y = x >> 1; if (y != 0) { return n - 2; }
        return n - x;
    }

    /*
     * @brief : Nのビットを一つ飛ばしにする(1111 -> 01010101)に変換
     */
    static uint64 AlternateBits(uint32 N) {
        uint64 X = N;
        X = (X | (X << 16)) & 0x0000ffff0000ffff;
        X = (X | (X << 8)) & 0x00ff00ff00ff00ff;
        X = (X | (X << 4)) & 0x0f0f0f0f0f0f0f0f;
        X = (X | (X << 2)) & 0x3333333333333333;
        return (X | (X << 1)) & 0x5555555555555555;
    }

    // nの偶数番目のビットのみ取り出して(最下位ビットは0番で偶数とする)詰めなおす
    // 例) 010101 => 111
    static uint32 ExtractEvenBits(uint64 N) {
        N = N & 0x5555555555555555;
        N = (N | (N >> 1)) & 0x3333333333333333;
        N = (N | (N >> 2)) & 0x0f0f0f0f0f0f0f0f;
        N = (N | (N >> 4)) & 0x00ff00ff00ff00ff;
        N = (N | (N >> 8)) & 0x0000ffff0000ffff;
        return static_cast<uint32>((N | (N >> 16)) & 0x00000000ffffffff);
    }

    // MortonNo -> X,Yを取り出す
    static void MortonNo2Cell(uint64 MortonNo, uint32& X, uint32& Y) {
        X = ExtractEvenBits(MortonNo);
        Y = ExtractEvenBits(MortonNo >> 1);
    }

    // (X, Y) -> MotionNoに変換する
    static uint64 Cell2MotionNo(uint32 X, uint32 Y) {
        return AlternateBits(X) | AlternateBits(Y) << 1;
    }


    /*
     * @brief : 二分探索. Predicate(Arr[i]) == false, Predicate(Arr[i+1]) == trueとなるiを返す
     *        : Arrは必ず[false, false, false, ....., true, true, true]とfalse, trueが分かれていないといけない(index若いほうがfalse)
     *        : レイ) LowerBound([0, 1, 2, 3, 3, 4, 5], [](auto x){ return x > 3;}) =>  4が返る
     *        : Arrがすべてfalseの場合 => Arr.Num() - 1
     *        : Arrがすべてtrueの場合  => -1
     *        : Arrが空の場合 => 0
     */
    template<class T, class F>
    static int LowerBound(const TArray<T>& Arr, F&& Predicate, int Min = -1, int Max = -1) {
        if (Arr.Num() == 0)
            return 0;
        if (Predicate(Arr[0]) == true)
            return -1;
        if (Predicate(Arr[Arr.Num() - 1]) == false)
            return Arr.Num() - 1;

        if (Min < 0)
            Min = 0;
        if (Max < 0)
            Max = Arr.Num() - 1;
        while (Min <= Max) {
            auto N = (Min + Max) / 2;
            if (Predicate(Arr[N])) {
                // 左側を探す
                Max = N - 1;
            }
            else {
                // 右側を探す
                Min = N + 1;
            }
        }
        return FMath::Clamp(Min - 1, -1, Arr.Num() - 1);
    }

    /*
     * @brief : PがSelfの辺上で最も近い位置を返す
     */
    static FVector2D GetClosestPointOnEdge(const FBox2D& Self, const FVector2D& P);

    /*
     * @brief : 矩形Pが, 矩形A, Bどちらとも重ならないような位置に移動したPを取得する
     */
    static FBox2D MoveBox2DToNonOverlapPoint(const FBox2D& P, const FBox2D& A, const FBox2D& B, const FBox2D& ScreenRange);
};
