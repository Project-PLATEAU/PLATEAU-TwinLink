#include "TwinLinkPLATEAUCityModelEx.h"

#include "Components/SceneComponent.h" 
#include "FTwinLinkEnumT.h"
#include "TwinLinkActorEx.h"

bool TwinLinkPLATEAUCityModelFindRequest::IsTargetMeshType(FTwinLinkFindCityModelMeshType MeshType) const {
    if (FTwinLinkEnumEx::IsValid(MeshType) == false)
        return false;
    const auto MeshTypeMask = static_cast<FTwinLinkFindCityModelMeshTypeMask>(1 << static_cast<uint8>(MeshType));
    return FTwinLInkBitFlagEx::ContainsAny(FindMeshTypeMask, MeshTypeMask);
}

bool TwinLinkPLATEAUCityModelFindRequest::IsTargetLod(int LodLevel, int MinLodLevel, int MaxLodLevel) const {
    if (FTwinLInkBitFlagEx::ContainsAny(FindLodTypeMask, FTwinLinkFindCityModelLodTypeMask::MMaxLod) && LodLevel == MaxLodLevel)
        return true;
    if (FTwinLInkBitFlagEx::ContainsAny(FindLodTypeMask, FTwinLinkFindCityModelLodTypeMask::MMinLod) && LodLevel == MinLodLevel)
        return true;
    return false;
}

bool TwinLinkPLATEAUInstancedCityModelIterator::CheckCityObjectGroup(
    const TArray<TObjectPtr<USceneComponent>>& CityObjectGroups, int& CityObjectGroupIndex) {
    while (CityObjectGroupIndex >= 0 && CityObjectGroupIndex < CityObjectGroups.Num() && Cast<UPLATEAUCityObjectGroup>(CityObjectGroups[CityObjectGroupIndex]) == nullptr) {
        CityObjectGroupIndex++;
    }

    return CityObjectGroupIndex < CityObjectGroups.Num();
}

bool TwinLinkPLATEAUInstancedCityModelIterator::CheckLod(const TArray<TObjectPtr<USceneComponent>>& Lods, int& LodIndex,
    int& CityObjectGroupIndex) {
    while (LodIndex >= 0 && LodIndex < Lods.Num() && CheckCityObjectGroup(Lods[LodIndex]->GetAttachChildren(), CityObjectGroupIndex) == false) {
        LodIndex++;
        CityObjectGroupIndex = 0;
    }
    return LodIndex < Lods.Num();
}

bool TwinLinkPLATEAUInstancedCityModelIterator::CheckChildren(const TArray<TObjectPtr<USceneComponent>>& Children,
    int& ChildIndex, int& LodIndex, int& CityObjectGroupIndex) {
    while (ChildIndex >= 0 && ChildIndex < Children.Num() && CheckLod(Children[ChildIndex]->GetAttachChildren(), LodIndex, CityObjectGroupIndex) == false) {
        ChildIndex++;
        LodIndex = CityObjectGroupIndex = 0;
    }
    return ChildIndex < Children.Num();
}

TwinLinkPLATEAUInstancedCityModelIterator TwinLinkPLATEAUInstancedCityModelIterator::operator++(int) {
    const auto Ret = *this;
    this->operator++();
    return Ret;
}

TwinLinkPLATEAUInstancedCityModelIterator& TwinLinkPLATEAUInstancedCityModelIterator::operator++() {
    CityObjectGroupIndex++;
    Check();
    return *this;
}

bool TwinLinkPLATEAUInstancedCityModelIterator::operator==(const TwinLinkPLATEAUInstancedCityModelIterator& Other) const {
    return Target == Other.Target &&
        ChildIndex == Other.ChildIndex &&
        LodIndex == Other.LodIndex &&
        CityObjectGroupIndex == Other.CityObjectGroupIndex;
}

bool TwinLinkPLATEAUInstancedCityModelIterator::operator!=(const TwinLinkPLATEAUInstancedCityModelIterator& Other) const {
    return !(*this == Other);
}

FTwinLinkCityObjectGroupModel TwinLinkPLATEAUInstancedCityModelIterator::operator*() const {
    FTwinLinkCityObjectGroupModel Ret;
    FTwinLinkPlateauCityModelEx::TryParseLodLevel(TwinLinkActorEx::GetChild(Target.Get(), { ChildIndex, LodIndex })->GetName(), Ret.LodLevel);
    Ret.CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(TwinLinkActorEx::GetChild(Target.Get(), { ChildIndex, LodIndex, CityObjectGroupIndex }));
    Ret.MeshType = FTwinLinkFindCityModelMeshType::Undefined;
    if (Ret.CityObjectGroup.IsValid())
        Ret.MeshType = FTwinLinkPlateauCityModelEx::ParseMeshType(Ret.CityObjectGroup->GetName());
    return Ret;;
}

TwinLinkPLATEAUInstancedCityModelIterator::TwinLinkPLATEAUInstancedCityModelIterator(TWeakObjectPtr<APLATEAUInstancedCityModel> CityModel, int ChildI, int LodI,
    int ObjectI)
    : Target(CityModel->GetRootComponent())
    , ChildIndex(ChildI)
    , LodIndex(LodI)
    , CityObjectGroupIndex(ObjectI) {
    Check();
}

void TwinLinkPLATEAUInstancedCityModelIterator::Check() {
    auto& Children = Target->GetAttachChildren();
    // すでに範囲外なら何もしない
    if (ChildIndex >= Children.Num())
        return;

    auto& Child = Children[ChildIndex];
    auto& Lods = Child->GetAttachChildren();
    auto NextChildren = [this, &Children]() {
        ChildIndex++;
        LodIndex = CityObjectGroupIndex = 0;
        CheckChildren(Children, ChildIndex, LodIndex, CityObjectGroupIndex);
    };
    // 本来ありえないがこの段階でLodが範囲外ならChildを進めて正しい方向に修正する
    if (LodIndex >= Lods.Num()) {
        NextChildren();
        return;
    }

    auto& Lod = Lods[LodIndex];
    auto& CityObjectGroups = Lod->GetAttachChildren();
    // 現在のCItyObjectGroupが正しいかどうかチェックする
    if (CheckCityObjectGroup(CityObjectGroups, CityObjectGroupIndex))
        return;

    // 正しくない場合はLodを進めてチェックする
    LodIndex++;
    CityObjectGroupIndex = 0;
    if (CheckLod(Lods, LodIndex, CityObjectGroupIndex))
        return;

    // それも範囲外の場合はChildも進めてチェックする
    NextChildren();
}


TwinLinkPLATEAUInstancedCityModelIterator::TwinLinkPLATEAUInstancedCityModelIterator() {}

TwinLinkPLATEAUInstancedCityModelScanner::TwinLinkPLATEAUInstancedCityModelScanner(
    TWeakObjectPtr<APLATEAUInstancedCityModel> CityModel) : Target(CityModel) {
}

TwinLinkPLATEAUInstancedCityModelIterator TwinLinkPLATEAUInstancedCityModelScanner::begin() const {
    return TwinLinkPLATEAUInstancedCityModelIterator(Target, 0, 0, 0);
}

TwinLinkPLATEAUInstancedCityModelIterator TwinLinkPLATEAUInstancedCityModelScanner::end() const {
    return TwinLinkPLATEAUInstancedCityModelIterator(Target, Target->GetRootComponent()->GetAttachChildren().Num(), 0, 0);
}

const TArray<char*> FTwinLinkPlateauCityModelEx::GetComponentNamePrefixes(FTwinLinkFindCityModelMeshType Type) {
    switch (Type) {
    case FTwinLinkFindCityModelMeshType::Bldg:
        return { "bldg_", "BLD_" };
    case FTwinLinkFindCityModelMeshType::Dem:
        return { "dem_", "DEM_" };
    case FTwinLinkFindCityModelMeshType::Tran:
        return { "tran_", "LND_" };
    case FTwinLinkFindCityModelMeshType::Urf:
        return { "urf_" };
    case FTwinLinkFindCityModelMeshType::Max:
    case FTwinLinkFindCityModelMeshType::Undefined:
    default:;
    }
    return { "__invalid__" };
}

FTwinLinkFindCityModelMeshType FTwinLinkPlateauCityModelEx::ParseMeshType(const FString& MeshTypeName) {
    for (auto I = 0; I < static_cast<int>(FTwinLinkFindCityModelMeshType::Max); ++I) {
        const auto Type = static_cast<FTwinLinkFindCityModelMeshType>(I);
        for (const auto& Prefix : GetComponentNamePrefixes(Type)) {
            if (MeshTypeName.StartsWith(Prefix))
                return Type;
        }
    }
    return FTwinLinkFindCityModelMeshType::Undefined;
}

bool FTwinLinkPlateauCityModelEx::TryParseLodLevel(const FString& LodName, int& OutLodLevel) {
    OutLodLevel = -1;

    // LODで始まっているかチェックする
    constexpr char LodPrefix[] = "LOD";
    constexpr int LodPrefixLength = sizeof(LodPrefix) - 1;
    if (LodName.Len() <= LodPrefixLength)
        return false;
    if (LodName.StartsWith(LodPrefix) == false)
        return false;

    auto Level = 0;
    // LOD以降で数字の部分を抜き出す
    for (auto I = LodPrefixLength; I < LodName.Len(); ++I) {
        // #TODO : マルチバイト文字は考慮していない
        // 数字かどうか判定する
        if (!std::isdigit(LodName[I]))
            break;
        Level = 10 * Level + (LodName[I] - '0');
    }
    // 数字が存在しているかチェックする
    if (Level == 0)
        return false;
    OutLodLevel = Level;
    return true;
}

bool FTwinLinkPlateauCityModelEx::TryGetMinMaxLodLevel(const USceneComponent* MeshRootComponent, int& OutMinLodLevel,
    int& OutMaxLodLevel) {
    if (!MeshRootComponent)
        return false;
    OutMinLodLevel = -1;
    OutMaxLodLevel = -1;
    for (auto Lod : MeshRootComponent->GetAttachChildren()) {
        FString LodName;
        Lod->GetName(LodName);
        int LodLevel = -1;
        if (TryParseLodLevel(LodName, LodLevel) == false)
            continue;
        if (OutMinLodLevel <= 0 || LodLevel < OutMinLodLevel)
            OutMinLodLevel = LodLevel;
        if (OutMaxLodLevel <= 0 || LodLevel > OutMaxLodLevel)
            OutMaxLodLevel = LodLevel;
    }
    // とりあえずどっちかだけ見ればよい
    return OutMinLodLevel > 0;
}
