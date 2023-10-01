#include "TwinLinkPLATEAUCityModelEx.h"

#include "FTwinLinkEnumT.h"

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

const char* TwinLinkPLATEAUCityModelEx::GetComponentNamePrefix(FTwinLinkFindCityModelMeshType Type) {
    switch (Type) {
    case FTwinLinkFindCityModelMeshType::Bldg:
        return "bldg";
    case FTwinLinkFindCityModelMeshType::Dem:
        return "dem";
    case FTwinLinkFindCityModelMeshType::Tran:
        return "tran";
    case FTwinLinkFindCityModelMeshType::Max:
    case FTwinLinkFindCityModelMeshType::Undefined:
    default:;
    }
    return "__invalid__";
}

FTwinLinkFindCityModelMeshType TwinLinkPLATEAUCityModelEx::ParseMeshType(const FString& MeshTypeName) {
    for (auto I = 0; I < static_cast<int>(FTwinLinkFindCityModelMeshType::Max); ++I) {
        const auto Type = static_cast<FTwinLinkFindCityModelMeshType>(I);
        if (MeshTypeName.Contains(GetComponentNamePrefix(Type)))
            return Type;
    }
    return FTwinLinkFindCityModelMeshType::Undefined;
}

bool TwinLinkPLATEAUCityModelEx::TryParseLodLevel(const FString& LodName, int& OutLodLevel) {
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

bool TwinLinkPLATEAUCityModelEx::TryGetMinMaxLodLevel(const USceneComponent* MeshRootComponent, int& OutMinLodLevel,
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
