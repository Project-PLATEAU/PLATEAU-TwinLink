// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorNavSystem.h"
#include <Kismet/GameplayStatics.h>
#include <optional>
#include <NavMesh/NavMeshBoundsVolume.h>
#include "Components/BrushComponent.h"
#include "PLATEAUInstancedCityModel.h"
#include "NavigationSystem.h"
#include "TwinLink/Public/TwinLinkNavSystem.h"
namespace
{
	template<class F>
	void ForeachDescendant(USceneComponent* Self, INT32 nest, F&& func) 
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
			ForeachDescendant(Child, nest - 1, std::forward<F>(func));
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
        for (const auto Comp : SceneComponents)
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
        for (const auto Child : ChildComponents)
            Child->SetCanEverAffectNavigation(false);
    }

    /*
    * @brief : CityModel
    */
    std::optional<FBox> ApplyNavMeshAffect(APLATEAUInstancedCityModel* Actor) {
        std::optional<FBox> Aabb = std::nullopt;
        if (!Actor)
            return Aabb;

        const auto RootComp = Actor->GetRootComponent();
        TArray<USceneComponent*> ChildComponent;
        RootComp->GetChildrenComponents(false, ChildComponent);

        ::ForeachDescendant(RootComp, 1,
            [&Aabb, Actor](USceneComponent* Self) mutable {
                FString MeshName;
                Self->GetName(MeshName);
                // tranという名前は道メッシュ
                if (MeshName.Contains("tran")) {
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
                            for (const auto Child : ChildComponents) {
                                const auto StaMeshComp = Cast<UStaticMeshComponent>(Child);
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

    std::optional<FBox> ApplyNavMeshAffect(const UWorld* World) {
        if (!World)
            return std::nullopt;
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

        std::optional<FBox> Ret = std::nullopt;
        for (const auto Actor : AllActors) {
            if (auto CityModel = Cast<APLATEAUInstancedCityModel>(Actor)) {
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

    /*
     * World->PersistentLevelの中からTでキャスト可能なものを線形探索
     *
     */
    template<class T>
    auto FindFirstPersistentLevelActor(UWorld* World) -> std::enable_if_t< std::is_base_of_v<AActor, T>, T* >
	{
        if (!World || !World->PersistentLevel)
            return nullptr;
        for(auto& Actor : World->PersistentLevel->Actors)
        {
            if (auto Ret = Cast<T>(Actor))
                return Ret;
        }
        return nullptr;
	}
}


void UTwinLinkEditorNavSystem::MakeNavMesh(UWorld* World)
{
    const auto BbBox = ::ApplyNavMeshAffect(World);
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, ANavMeshBoundsVolume::StaticClass(), AllActors);

    TArray<AActor*> NavSystemActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATwinLinkNavSystem::StaticClass(), NavSystemActors);

    const ATwinLinkNavSystem* NavSystemActor = nullptr;
    if (NavSystemActors.Num() > 0) {
        NavSystemActor = Cast<ATwinLinkNavSystem>(NavSystemActors[0]);
    }
    else {
        NavSystemActor = Cast<ATwinLinkNavSystem>(SpawnActorFromClass(ATwinLinkNavSystem::StaticClass(), FVector::Zero(), FRotator::ZeroRotator));
    }

	if (BbBox.has_value() == false)
	{
        for (const auto Actor : AllActors)
            Actor->Destroy();
	}
	else
	{
        ANavMeshBoundsVolume* Volume = nullptr;
        if (AllActors.Num() > 0)
            Volume = Cast<ANavMeshBoundsVolume>(AllActors[0]);
        else
            Volume = Cast<ANavMeshBoundsVolume>(
                SpawnActorFromClass(ANavMeshBoundsVolume::StaticClass(), FVector::Zero(), FRotator::ZeroRotator));

        const auto Center = BbBox->GetCenter();
        const auto Extent = BbBox->GetExtent();
        const auto BrushBounds = Volume->Brush->Bounds;

        const FVector Scale = Extent / BrushBounds.BoxExtent;
        Volume->SetActorLocation(Center);
        Volume->SetActorScale3D(Scale);
        if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
            NavSys->OnNavigationBoundsUpdated(Volume);
            NavSys->Build();
		}
	}
}
void UTwinLinkEditorNavSystem::SetCanEverAffectNavigationAllActors(UWorld* World, bool Relevant)
{
	if (!World)
		return;
     
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
	for (const auto Actor : Actors) {
        ::SetCanEverAffectNavigationRecursively(Actor, Relevant);
	}
}

FString UTwinLinkEditorNavSystem::GetNavMeshBuildingMessage(UWorld* World)
{
    // まだ実行すらしていない
    const auto NavSystem = ::FindFirstPersistentLevelActor<ATwinLinkNavSystem>(World);
    if (!NavSystem)
        return "";

    // モデルを読み込んでいない
    const auto CityModel = ::FindFirstPersistentLevelActor<APLATEAUInstancedCityModel>(World);
    if (!CityModel)
        return "City Model not found";

    //
    auto bExistLoadModel = false;
    ::ForeachDescendant(CityModel->GetRootComponent(), 1,
        [&bExistLoadModel](const USceneComponent* Self) mutable 
        {
            FString MeshName;
            Self->GetName(MeshName);
            // tranという名前は道メッシュ
            if (MeshName.Contains("tran") == false)
                return;

            for (auto Lod : Self->GetAttachChildren()) 
            {
                FString LodName;
                Lod->GetName(LodName);
                if (LodName.StartsWith("LOD1") == false)
                    continue;
                bExistLoadModel = true;
                return;
            }
        });
    // 道モデルが見つからない
    if (bExistLoadModel == false)
        return "Load Model not found";

    // https://docs.unrealengine.com/5.2/en-US/API/Runtime/NavigationSystem/UNavigationSystemV1/
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
        return "NavSystem not found";

    // https://docs.unrealengine.com/5.0/ja/optimizing-navigation-mesh-generation-speed-in-unreal-engine/
    // ナビメッシュ生成中
    if (NavSys->IsNavigationBuildInProgress()) 
    {
        // ...を描画するためのもの
        const auto TimeSec = World->GetTimeSeconds();
        const auto DotNum = ((int)TimeSec) % 3 + 1;
        FString ret = "Building";        
        ret.Append(FString::ChrN(DotNum, '.'));
        return ret;
    }
    if (NavSys->IsNavigationDirty())
        return "NavMesh Data is Dirty";
    if (NavSys->IsNavigationBuilt(World->GetWorldSettings()))
        return "Completed";
    return "Invalid Error";
}
