#pragma once
#include "PLATEAUInstancedCityModel.h"
class APlayerController;
class USceneComponent;
UENUM(BlueprintType)
enum class FTwinLinkFindCityModelMeshType : uint8 {
    // 建物
    Bldg UMETA(DisplayName = "Bldg(建物)"),
    // 道路
    Dem UMETA(DisplayName = "Dem(道路モデル)"),
    // 道(ナビメッシュ)
    Tran UMETA(DisplayName = "Tran(道路ナビメッシュ)"),
    // 道(ナビメッシュ)
    Urf UMETA(DisplayName = "Urf(都市計画決定情報)"),
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
    // 都市計画決定情報
    MUrf = (1 << static_cast<uint8>(FTwinLinkFindCityModelMeshType::Urf)) UMETA(DisplayName = "Urf(都市計画決定情報)"),

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

struct FTwinLinkCityObjectGroupModel {
public:
    TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
    FTwinLinkFindCityModelMeshType MeshType;
    int LodLevel;
};

/*
 * @brief : APLATEAUInstancedCityModelの中にあるUPLATEAUCityObjectGroupのイテレータ
 */
struct TWINLINK_API TwinLinkPLATEAUInstancedCityModelIterator {

public:
    // CityObjectGroupが見つかるまでCityObjectGroupIndexを進める(現在位置がすでにCityObjectGroupの場合は無視される)
    // true : 正常に進められた, false : 配列の終端に達した
    static bool CheckCityObjectGroup(const TArray<TObjectPtr<USceneComponent>>& CityObjectGroups, int& CityObjectGroupIndex);

    // CityObjectGroupが見つかるまでLodIndexを進める. CityObjectGroupIndexは見つかったときのCityObjectGroupのインデックスが入る(現在位置がすでにCityObjectGroupの場合は無視される
    // true : 正常に進められた, false : 配列の終端に達した
    static bool CheckLod(const TArray<TObjectPtr<USceneComponent>>& Lods, int& LodIndex, int& CityObjectGroupIndex);

    // CityObjectGroupが見つかるまでChildIndexを進める, LodIndex, CityObjectGroupIndexは見つかったときのLod,CityObjectGroupのインデックスが入る(現在位置がすでにCityObjectGroupの場合は無視される
    // true : 正常に進められた, false : 配列の終端に達した
    static bool CheckChildren(const TArray<TObjectPtr<USceneComponent>>& Children, int& ChildIndex, int& LodIndex, int& CityObjectGroupIndex);

public:
    TwinLinkPLATEAUInstancedCityModelIterator operator++(int);

    TwinLinkPLATEAUInstancedCityModelIterator& operator++();

    bool operator==(const TwinLinkPLATEAUInstancedCityModelIterator& Other) const;

    bool operator!=(const TwinLinkPLATEAUInstancedCityModelIterator& Other) const;

    FTwinLinkCityObjectGroupModel operator*() const;
    TwinLinkPLATEAUInstancedCityModelIterator();
    TwinLinkPLATEAUInstancedCityModelIterator(TWeakObjectPtr<APLATEAUInstancedCityModel> CityModel, int ChildI, int LodI, int ObjectI);
private:
    // 現在指しているオブジェクトが正しいものになるまで進める
    void Check();
private:
    // APLATEAUInstancedCityModelのRootComponent
    TWeakObjectPtr<USceneComponent> Target = nullptr;
    int ChildIndex = 0;
    int LodIndex = 0;
    int CityObjectGroupIndex = 0;
    FTwinLinkFindCityModelMeshType NowChildMeshType = FTwinLinkFindCityModelMeshType::Undefined;
};
/*
 * @brief : APLATEAUInstancedCityModelの中にあるUPLATEAUCityObjectGroupを走査するためのコンテナ
 */
class TWINLINK_API TwinLinkPLATEAUInstancedCityModelScanner {
public:
    TwinLinkPLATEAUInstancedCityModelScanner(TWeakObjectPtr<APLATEAUInstancedCityModel> CityModel);
    TwinLinkPLATEAUInstancedCityModelIterator begin() const;
    TwinLinkPLATEAUInstancedCityModelIterator end() const;
private:
    TWeakObjectPtr<APLATEAUInstancedCityModel> Target;
};

class TWINLINK_API FTwinLinkPlateauCityModelEx {
public:
    /*
     * @brief: Typeに応じたコンポーネントの名前のプリフィックスを返す. 不正値の場合は__invalid__が返る
     */
    static const TArray<char*> GetComponentNamePrefixes(FTwinLinkFindCityModelMeshType Type);

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
            // RootComponentの直下のコンポーネントの名前は任意なので判断できない


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
