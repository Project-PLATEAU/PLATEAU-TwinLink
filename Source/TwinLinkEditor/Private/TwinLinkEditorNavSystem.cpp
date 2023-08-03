// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorNavSystem.h"
#include <Kismet/GameplayStatics.h>
#include <optional>
#include <NavMesh/NavMeshBoundsVolume.h>
#include "Components/BrushComponent.h"
#include "PLATEAUInstancedCityModel.h"
#include "NavigationSystem.h"
namespace
{
	template<class F>
	void ForeachDesendant(USceneComponent* Self, INT32 nest, F&& func) 
	{
		if (nest < 0)
			return;
		if (nest == 0)
		{
			func(Self);
			return;
		}
		for (auto Child : Self->GetAttachChildren())
		{
			ForeachDesendant(Child, nest - 1, std::forward<F>(func));
		}
	}

    /*
    * @brief : Actor以下の全ComponentのSetCanEverAffectNavigationを設定する
    */
    void SetCanEverAffectNavigationRecursively(AActor* Actor, bool Relevant) {
        if (!Actor)
            return;

        TArray<USceneComponent*> SceneComponents;
        Actor->GetComponents(SceneComponents);
        for (auto Comp : SceneComponents)
            Comp->SetCanEverAffectNavigation(Relevant);
    }

    /*
    * @brief : Comp以下の全ComponentのSetCanEverAffectNavigationを設定する
    */
    void SetCanEverAffectNavigationRecursively(USceneComponent* Comp, bool Relevant) {
        if (!Comp)
            return;
        Comp->SetCanEverAffectNavigation(false);
        TArray<USceneComponent*> ChildComponents;
        Comp->GetChildrenComponents(true, ChildComponents);
        for (auto Child : ChildComponents)
            Child->SetCanEverAffectNavigation(false);
    }

    /*
    * @brief : CityModel
    */
    std::optional<FBox> ApplyNavMeshAffect(APLATEAUInstancedCityModel* Actor) {
        std::optional<FBox> Aabb = std::nullopt;
        if (!Actor)
            return Aabb;

        auto RootComp = Actor->GetRootComponent();
        TArray<USceneComponent*> ChildComponent;
        RootComp->GetChildrenComponents(false, ChildComponent);

        ::ForeachDesendant(RootComp, 1,
            [&Aabb, Actor](USceneComponent* Self) mutable {
                FString MeshName;
                Self->GetName(MeshName);
                // tranという名前は道メッシュ
                const auto IsRoad = MeshName.Contains("tran");
                if (IsRoad) {
                    // tran以下は以下のようにLODごとに階層分けされている
                    // https://synesthesias.atlassian.net/browse/KUKANIDBIM-43
                    // ナビメッシュの対象になっているのはLOD1のみ
                    // LOD1 xxxx
                    //    - xxxx_tran_yyy
                    //    - xxxx_tran_zzz 
                    // 自分自身もオフにしておく
                    Self->SetCanEverAffectNavigation(false);
                    for (auto Lod : Self->GetAttachChildren()) {
                        FString LodName;
                        Lod->GetName(LodName);
                        if (LodName.StartsWith("LOD1")) {
                            TArray<USceneComponent*> ChildComponents;
                            Lod->GetChildrenComponents(true, ChildComponents);
                            for (auto Child : ChildComponents) {
                                auto StaMeshComp = Cast<UStaticMeshComponent>(Child);
                                if (!StaMeshComp)
                                    continue;
                                StaMeshComp->SetCanEverAffectNavigation(true);
                                const auto StaMesh = StaMeshComp->GetStaticMesh();
                                if (!StaMesh)
                                    continue;
                                auto Bounds = StaMesh->GetBounds();
                                auto Bb = Bounds.GetBox();
                                auto Center = Bb.GetCenter();
                                // 最低1mは用意する(tranが平面ポリゴンでbbのzが0になることがあるので
                                auto Extent = FVector::Max(Bb.GetExtent(), FVector::One() * 100);
                                Bb = FBox(Center - Extent, Center + Extent);
                                if (Aabb.has_value() == false) {
                                    Aabb = Bb;
                                }
                                else {
                                    // 領域を大きくする
                                    Aabb = *Aabb + Bb;
                                }
                            }
                        }
                        // それ以外はオフにする
                        else {
                            SetCanEverAffectNavigationRecursively(Lod, false);
                        }
                    }
                }
                else {
                    // すべて無効にする
                    SetCanEverAffectNavigationRecursively(Self, false);
                }
            });
        return Aabb;
    }

    std::optional<FBox> ApplyNavMeshAffect(UWorld* world) {
        if (!world)
            return std::nullopt;
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(world, AActor::StaticClass(), AllActors);

        std::optional<FBox> Ret = std::nullopt;
        for (auto Actor : AllActors) {
            auto CityModel = Cast<APLATEAUInstancedCityModel>(Actor);
            if (CityModel) {
                auto Aabb = ApplyNavMeshAffect(Cast<APLATEAUInstancedCityModel>(Actor));
                if (!Aabb.has_value())
                    continue;
                if (Ret.has_value() == false) {
                    Ret = Aabb;
                }
                else {
                    Ret = *Aabb + *Ret;
                }
            }
            else {
                // すべて無効にする
                SetCanEverAffectNavigationRecursively(Actor, false);
            }
        }
        return Ret;
    }
}


void UTwinLinkEditorNavSystem::MakeNavMesh(UWorld* world)
{
    auto BbBox = ::ApplyNavMeshAffect(world);
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(world, ANavMeshBoundsVolume::StaticClass(), AllActors);
    
	if (BbBox.has_value() == false)
	{
        for (auto Actor : AllActors)
            Actor->Destroy();
	}
	else
	{
        ANavMeshBoundsVolume* Volume = nullptr;
        if (AllActors.Num() > 0)
            Volume = Cast<ANavMeshBoundsVolume>(AllActors[0]);
        else
            Volume = Cast<ANavMeshBoundsVolume>(SpawnActorFromClass(ANavMeshBoundsVolume::StaticClass(), FVector::Zero(), FRotator::ZeroRotator));

		auto center = BbBox->GetCenter();
		auto extent = BbBox->GetExtent();
		auto brush_bounds = Volume->Brush->Bounds;

		FVector scale = extent / brush_bounds.BoxExtent;
        Volume->SetActorLocation(center);
        Volume->SetActorScale3D(scale);
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(world);
		if (NavSys)
		{
			NavSys->OnNavigationBoundsUpdated(Volume);
		}
	}
}
void UTwinLinkEditorNavSystem::SetCanEverAffectNavigationAllActors(UWorld* World, bool Relevant)
{
	if (!World)
		return;
     
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
	for (auto Actor : Actors) {
        ::SetCanEverAffectNavigationRecursively(Actor, Relevant);
	}
}