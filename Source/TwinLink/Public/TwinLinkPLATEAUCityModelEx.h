#pragma once
#include "PLATEAUInstancedCityModel.h"
class APlayerController;

UENUM(BlueprintType)
enum class FTwinLinkFindCityModelMeshType : uint8 {
    // 建物
    Bldg UMETA(DisplayName = "Bldg(建物)"),
    // 道路
    Dem UMETA(DisplayName = "Dem(道路モデル)"),
    // 道(ナビメッシュ)
    Tran UMETA(DisplayName = "Tran(道路ナビメッシュ)"),
    // 要素数
    Max UMETA(DisplayName = "Max(要素数を表す予約語)"),
    // 不正値
    Undefined UMETA(DisplayName = "Undefined(不正値を表す予約語)")
};

UENUM(BlueprintType, Meta = (Bitflags))
enum class FTwinLinkFindCityModelMeshTypeMask : uint8 {
    // 建物
    MBldg = (1 << static_cast<uint8>(FTwinLinkFindCityModelMeshType::Bldg)) UMETA(DisplayName = "Bldg(建物)"),
    // 道路
    MDem = (1 << static_cast<uint8>(FTwinLinkFindCityModelMeshType::Dem)) UMETA(DisplayName = "Dem(道路モデル)"),
    // 道(ナビメッシュ)
    MTran = (1 << static_cast<uint8>(FTwinLinkFindCityModelMeshType::Tran)) UMETA(DisplayName = "Tran(道路ナビメッシュ)"),

    // 空マスク
    Empty = 0 UMETA(DisplayName = "Empty(空)"),
    // 全マスク
    All = ((1 << static_cast<uint8>(FTwinLinkFindCityModelMeshType::Max)) - 1)  UMETA(DisplayName = "All(すべて)")
};

UENUM(BlueprintType)
enum class FTwinLinkFindCityModelLodType : uint8 {
    // 最大LOD
    MaxLod UMETA(DisplayName = "MaxLod(最大LOD)"),
    // 最小LOD
    MinLod UMETA(DisplayName = "MinLod(最小LOD)"),
    // 個数
    Max UMETA(DisplayName = "Max(個数を表す予約語)"),
    // 不正値
    Undefined UMETA(DisplayName = "Undefined(不正値を表す予約語)")
};

UENUM(BlueprintType)
enum class FTwinLinkFindCityModelLodTypeMask : uint8 {
    // 最大LOD
    MMaxLod = (1 << static_cast<uint8>(FTwinLinkFindCityModelLodType::MaxLod)) UMETA(DisplayName = "MaxLod(最大LOD)"),
    // 最小LOD
    MMinLod = (1 << static_cast<uint8>(FTwinLinkFindCityModelLodType::MinLod)) UMETA(DisplayName = "MinLod(最小LOD)"),
    // 空マスク
    Empty = 0 UMETA(DisplayName = "Empty(空)"),
    // 全マスク
    All = ((1 << static_cast<uint8>(FTwinLinkFindCityModelLodType::Max)) - 1)  UMETA(DisplayName = "All(すべて)")
};
struct TwinLinkPLATEAUCityModelFindRequest {
    // 対象のメッシュタイプかどうか
    bool IsTargetMeshType(FTwinLinkFindCityModelMeshType MeshType) const;

    // 対象のLODレベルかどうか
    bool IsTargetLod(int LodLevel, int MinLodLevel, int MaxLodLevel) const;

    FTwinLinkFindCityModelLodTypeMask FindLodTypeMask;
    FTwinLinkFindCityModelMeshTypeMask FindMeshTypeMask;
};

class TWINLINK_API TwinLinkPLATEAUInstancedCityModelVisitor {
    static bool NextCityObjectGroup(const TArray<TObjectPtr<USceneComponent>>& CityObjectGroups, int& CityObjectGroupIndex) {
        do {
            CityObjectGroupIndex++;
        } while (CityObjectGroupIndex < CityObjectGroups.Num() && Cast<UPLATEAUCityObjectGroup>(CityObjectGroups[CityObjectGroupIndex]) == nullptr);

        return CityObjectGroupIndex >= CityObjectGroups.Num();
    }

    static bool NextLod(const TArray<TObjectPtr<USceneComponent>>& Lods, int& LodIndex, int& CityObjectGroupIndex) {
        do {
            LodIndex++;
            CityObjectGroupIndex = 0;
        } while (LodIndex < Lods.Num() && NextCityObjectGroup(Lods[LodIndex]->GetAttachChildren(), CityObjectGroupIndex));
        return LodIndex < Lods.Num();
    }
public:
    struct Iterator {
        Iterator operator++(int) {
            auto Ret = *this;
            this->operator++();
            return Ret;
        }

        Iterator& operator++() {
            auto& Children = Self->GetAttachChildren();
            // すでに範囲外の場合は何もしない
            while (ChildIndex < Children.Num()) {
                auto& Child = Children[ChildIndex];
                auto& Lods = Child->GetAttachChildren();
                auto& Lod = Lods[LodIndex];
                auto& CityObjectGroups = Lod->GetAttachChildren();
                if (NextCityObjectGroup(CityObjectGroups, CityObjectGroupIndex) == false)
                    return *this;

                if (NextLod(Lods, LodIndex, CityObjectGroupIndex) == false)
                    return *this;

                ChildIndex++;
                LodIndex = CityObjectGroupIndex = 0;
            }
            return *this;
        }

        bool operator==(const Iterator& Other) const {
            return Self == Other.Self &&
                ChildIndex == Other.ChildIndex &&
                LodIndex == Other.LodIndex &&
                CityObjectGroupIndex == Other.CityObjectGroupIndex;
        }

        Iterator() {}

        Iterator(TWeakObjectPtr<USceneComponent> CityModel)
            : Self(CityModel) {
        }

        Iterator(TWeakObjectPtr<USceneComponent> CityModel, int Index)
            : Self(CityModel)
            , ChildIndex(Index) {
        }

        UPLATEAUCityObjectGroup* operator*(){
            return GetCityObjectGroup();
        }

        UPLATEAUCityObjectGroup* GetCityObjectGroup() {
            return Cast<UPLATEAUCityObjectGroup>(GetChild(Self.Get(), { ChildIndex, LodIndex, CityObjectGroupIndex }));
        }
    private:
        TWeakObjectPtr<USceneComponent> Self;
        int ChildIndex = 0;
        int LodIndex = 0;
        int CityObjectGroupIndex = 0;
    };
public:
    TwinLinkPLATEAUInstancedCityModelVisitor(TWeakObjectPtr<APLATEAUInstancedCityModel> CityModel)
        : Target(CityModel) {
    }
    static USceneComponent* GetChildImpl(USceneComponent* Self, const int* now, const int* end) {
        if (now == end)
            return Self;

        auto& Children = Self->GetAttachChildren();
        if (*now >= Children.Num())
            return nullptr;

        return GetChildImpl(Children[*now], ++now, end);
    }

    static USceneComponent* GetChild(USceneComponent* Self, const std::initializer_list<int>& Indices) {
        return GetChildImpl(Self, Indices.begin(), Indices.end());
    }
    Iterator begin() const {
        // 0番目の要素が正しいかチェックする
        const auto Comp = Target->GetRootComponent();
        if (Cast<UPLATEAUCityObjectGroup>(GetChild(Comp, { 0, 0, 0 })))
            return Iterator(Comp);

        Iterator Ret(Comp);
        ++Ret;
        return Ret;
    }

    Iterator end() const {
        const auto Comp = Target->GetRootComponent();
        return Iterator(Comp, Comp->GetAttachChildren().Num());
    }
private:
    TWeakObjectPtr<APLATEAUInstancedCityModel> Target;
};

class TWINLINK_API TwinLinkPLATEAUCityModelEx {
public:
    /*
     * @brief: Typeに応じたコンポーネントの名前のプリフィックスを返す. 不正値の場合は__invalid__が返る
     */
    static const char* GetComponentNamePrefix(FTwinLinkFindCityModelMeshType Type);

    /*
     * @brief : コンポーネント名からメッシュタイプを取得する
     */
    static FTwinLinkFindCityModelMeshType ParseMeshType(const FString& MeshTypeName);

    // LODx_yyという名前からxを抜き出す
    static bool TryParseLodLevel(const FString& LodName, int& OutLodLevel);

    // 直下の子コンポーネントの名前を見て最大LODと最小LODをとってくる
    static bool TryGetMinMaxLodLevel(const USceneComponent* MeshRootComponent, int& OutMinLodLevel, int& OutMaxLodLevel);

    template<class F>
    static void ForeachModels(const APLATEAUInstancedCityModel* Self
        , const TwinLinkPLATEAUCityModelFindRequest& Request
        , F&& Func
    ) {
        if (!Self)
            return;
        const auto Root = Self->GetRootComponent();
        if (!Root)
            return;
        for (auto& Child : Root->GetAttachChildren()) {
            FString ChildName;
            Child->GetName(ChildName);
            // メッシュで判定する
            const auto MeshType = ParseMeshType(ChildName);
            if (Request.IsTargetMeshType(MeshType) == false)
                continue;

            // LODレベルを取ってくる
            int MaxLodLevel = -1;
            int MinLodLevel;
            if (TryGetMinMaxLodLevel(Child, MinLodLevel, MaxLodLevel) == false)
                continue;

            for (auto& Lod : Child->GetAttachChildren()) {
                FString LodName;
                Lod->GetName(LodName);
                int LodLevel;
                if (TryParseLodLevel(LodName, LodLevel) == false)
                    continue;

                if (Request.IsTargetLod(LodLevel, MinLodLevel, MaxLodLevel) == false)
                    continue;

                for (auto& Obj : Lod->GetAttachChildren()) {
                    const auto CityObj = Cast<UPLATEAUCityObjectGroup>(Obj);
                    if (!CityObj)
                        continue;
                    Func(CityObj);
                }
            }
        }
    }

};