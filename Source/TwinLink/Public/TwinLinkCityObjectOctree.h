#pragma once
#include "PLATEAUInstancedCityModel.h"
#include "TwinLinkPLATEAUCityModelEx.h"
#include "Math/GenericOctree.h"
#include <optional>
class APlayerController;
class USceneComponent;
class APLATEAUInstancedCityModel;
/*
 * @brief : APLATEAUInstancedCityModelの中にあるUPLATEAUCityObjectGroupのイテレータ
 */
struct TWINLINK_API TwinLinkCityObjectTree {

    struct Node {
        FVector GetPosition() const;
        FVector GetExtent() const;
        TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
    };

    // TOctree2に渡すためのセマンティッククラス
    struct Semantics {
        enum { MaxElementsPerLeaf = 16 };
        enum { MinInclusiveElementsPerNode = 7 };
        enum { MaxNodeDepth = 12 };

        typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

        /*
         * @brief : Nodeからバウンディングボックスを取得する
         */
        FORCEINLINE static FBoxCenterAndExtent GetBoundingBox(const Node& N) {
            return FBoxCenterAndExtent(N.GetPosition(), N.GetExtent());
        }

        /**
         * @brief: Nodeの==を定義
         */
        FORCEINLINE static bool AreElementsEqual(const Node& A, const Node& B) {
            return A.CityObjectGroup == B.CityObjectGroup;
        }

        FORCEINLINE static void SetElementId(const Node& Element, FOctreeElementId2 Id) {
        }
    };

    void Init(APLATEAUInstancedCityModel* CityModel);

    void Add(TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObject);

    using TreeType = TOctree2<Node, Semantics>;
    TreeType*Tree = nullptr;
    TWeakObjectPtr<APLATEAUInstancedCityModel> InstancedCityModel;
};
