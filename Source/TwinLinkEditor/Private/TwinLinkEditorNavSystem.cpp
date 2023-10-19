// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorNavSystem.h"

#include <algorithm>
#include <Kismet/GameplayStatics.h>
#include <optional>
#include <NavMesh/NavMeshBoundsVolume.h>
#include "PLATEAUInstancedCityModel.h"
#include "NavigationSystem.h"
#include "PLATEAUCityModelLoader.h"
#include "TwinLink.h"
#include "TwinLinkPLATEAUCityModelEx.h"
#include "TwinLink/Public/NavSystem/TwinLinkNavSystem.h"
namespace {
    /*
     * SelfからNest番目の子に対してFuncを実行する
     */
    template<class F>
    void ForeachDescendant(USceneComponent* Self, INT32 Nest, F&& Func) {
        if (Nest < 0)
            return;
        if (Nest == 0) {
            Func(Self);
            return;
        }
        for (auto Child : Self->GetAttachChildren()) {
            ForeachDescendant(Child, Nest - 1, std::forward<F>(Func));
        }
    }

    /*
     * Actorの持っている全コンポーネントに対してFuncを実行する
     */
    template<class F>
    void ForeachComponents(AActor* Actor, F&& Func) {
        if (!Actor)
            return;
        TArray<USceneComponent*> SceneComponents;
        Actor->GetComponents(SceneComponents);
        for (const auto Comp : SceneComponents)
            Func(Comp);
    }

    /*
     * Compの自信を含む全子コンポーネントに対してFuncを実行する
     */
    template<class F>
    void ForeachChildComponents(USceneComponent* Comp, F&& Func) {
        if (!Comp)
            return;
        Func(Comp);
        TArray<USceneComponent*> ChildComponents;
        Comp->GetChildrenComponents(true, ChildComponents);
        for (const auto Child : ChildComponents)
            Func(Child);
    }

    /*
    * @brief : Actor以下の全ComponentのSetCanEverAffectNavigationを設定する
    */
    void SetCanEverAffectNavigationRecursively(AActor* Actor, bool bRelevant) {
        if (!Actor)
            return;
        ForeachComponents(Actor, [bRelevant](USceneComponent* Child) {
            Child->SetCanEverAffectNavigation(bRelevant);
            });
    }

    /*
    * @brief : Comp以下の全ComponentのSetCanEverAffectNavigationを設定する
    */
    void SetCanEverAffectNavigationRecursively(USceneComponent* Comp, bool bRelevant) {
        if (!Comp)
            return;
        ForeachChildComponents(Comp, [bRelevant](USceneComponent* Child) {
            Child->SetCanEverAffectNavigation(bRelevant);
            });
    }

    /*
     * @brief : ナビメッシュ生成のリクエスト
     */
    struct ApplyNavMeshAffectRequest {
        // 対象のアクタ
        APLATEAUInstancedCityModel* Actor = nullptr;
        // ランタイムの道路モデルで扱うコリジョンチャンネル
        ECollisionChannel DemCollisionChannel = ECC_GameTraceChannel1;
    };

    /*
     * @brief : ナビメッシュ生成の結果
     */
    struct ApplyNavMeshAffectResult {
        ApplyNavMeshAffectResult() {};
        std::optional<FBox> NavVolumeAabb = std::nullopt;
        std::optional<FBox> DemAabb = std::nullopt;
    };

    /*
     * Selfに対してBbで領域を大きくする
     */
    void ExpandFBox(std::optional<FBox>& Self, const FBox& Bb) {
        if (Self.has_value() == false) {
            Self = Bb;
        }
        else {
            // 領域を大きくする
            Self = *Self + Bb;
        }
    }

    /*
    * @brief : CityModel
    */
    std::optional<ApplyNavMeshAffectResult> ApplyNavMeshAffect(const ApplyNavMeshAffectRequest& Request) {
        if (!Request.Actor)
            return std::nullopt;
        ApplyNavMeshAffectResult Result;
        // 道モデルのナビメッシュを作成する
        auto ApplyTran = [&Result](USceneComponent* Self) {
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
                        ExpandFBox(Result.NavVolumeAabb, Bb);
                    }
                }
                // それ以外はオフにする
                else {
                    SetCanEverAffectNavigationRecursively(Lod, false);
                }
            }
        };

        auto ApplyDem = [&Result, &Request](USceneComponent* Self) {
            TArray<USceneComponent*> ChildComponents;
            Self->GetChildrenComponents(true, ChildComponents);
            for (const auto Child : ChildComponents) {
                const auto StaMeshComp = Cast<UStaticMeshComponent>(Child);
                if (!StaMeshComp)
                    continue;
                StaMeshComp->SetCollisionResponseToChannel(Request.DemCollisionChannel, ECR_Block);
                const auto StaMesh = StaMeshComp->GetStaticMesh();
                if (!StaMesh)
                    continue;
                auto Bounds = StaMesh->GetBounds();
                auto Bb = Bounds.GetBox();
                auto Center = Bb.GetCenter();
                // 最低1mは用意する(tranが平面ポリゴンでbbのzが0になることがあるので
                auto Extent = FVector::Max(Bb.GetExtent(), FVector::One() * 100);
                Bb = FBox(Center - Extent, Center + Extent);
                ExpandFBox(Result.DemAabb, Bb);
            }
        };
        const auto RootComp = Request.Actor->GetRootComponent();
        ::ForeachDescendant(RootComp, 1,
            [&Result, &Request, &ApplyTran, &ApplyDem](USceneComponent* Self) mutable {
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
                    ApplyTran(Self);
                }
                else {
                    // すべて無効にする
                    SetCanEverAffectNavigationRecursively(Self, false);
                }

                // 道路自体のメッシュモデル
                if (MeshName.Contains("dem")) {
                    ApplyDem(Self);
                }
                else {
                    ForeachChildComponents(Self, [&Request](USceneComponent* Child) {
                        if (const auto StaMeshComp = Cast<UStaticMeshComponent>(Child)) {
                            StaMeshComp->SetCollisionResponseToChannel(Request.DemCollisionChannel, ECR_Ignore);
                        }
                        });
                }
            });
        return Result;
    }

    std::optional<ApplyNavMeshAffectResult> ApplyNavMeshAffect(const UWorld* World, ATwinLinkNavSystem* NavSystem) {
        if (!World)
            return std::nullopt;
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

        ApplyNavMeshAffectResult Ret;
        for (const auto Actor : AllActors) {
            if (auto CityModel = Cast<APLATEAUInstancedCityModel>(Actor)) {
                ApplyNavMeshAffectRequest Request{ CityModel, NavSystem->GetDemCollisionChannel() };
                auto Result = ApplyNavMeshAffect(Request);
                if (Result.has_value() == false)
                    continue;
                if (Result->NavVolumeAabb.has_value())
                    ExpandFBox(Ret.NavVolumeAabb, *Result->NavVolumeAabb);
                if (Result->DemAabb.has_value())
                    ExpandFBox(Ret.DemAabb, *Result->DemAabb);
            }
            else {
                // すべて無効にする
                SetCanEverAffectNavigationRecursively(Actor, false);
            }
        }

        if (Ret.DemAabb.has_value())
            NavSystem->SetDemCollisionAabb(*Ret.DemAabb);

        return Ret;
    }

    /*
     * World->PersistentLevelの中からTでキャスト可能なものを線形探索
     *
     */
    template<class T>
    auto FindFirstPersistentLevelActor(UWorld* World) -> std::enable_if_t< std::is_base_of_v<AActor, T>, T* > {
        if (!World || !World->PersistentLevel)
            return nullptr;
        for (auto& Actor : World->PersistentLevel->Actors) {
            if (auto Ret = Cast<T>(Actor))
                return Ret;
        }
        return nullptr;
    }

    struct NavMeshBuildProgress {
        // ナビメッシュビルド完了
        bool bIsBuildComplete;

        // 失敗 or 途中の場合のエラーメッセージ
        FString Message;

        NavMeshBuildProgress(bool bComplete, const FString& Msg)
            : bIsBuildComplete(bComplete)
            , Message(Msg) {
        }

        /*
         * エラー or 途中
         */
        static NavMeshBuildProgress Error(const FString& Msg) {
            return NavMeshBuildProgress(false, Msg);
        }

        /*
         * 成功
         */
        static NavMeshBuildProgress Complete() {
            return NavMeshBuildProgress(true, "");
        }

        static bool CheckCityModel(const APLATEAUInstancedCityModel* CityModel, FString& ErrMsg) {
            auto bExistRoadModel = false;
            ::ForeachDescendant(CityModel->GetRootComponent(), 1,
                [&bExistRoadModel](const USceneComponent* Self) mutable {
                    FString MeshName;
                    Self->GetName(MeshName);
                    // tranという名前は道メッシュ
                    if (MeshName.Contains("tran") == false)
                        return;

                    for (const auto Lod : Self->GetAttachChildren()) {
                        FString LodName;
                        Lod->GetName(LodName);
                        if (LodName.StartsWith("LOD1") == false)
                            continue;
                        bExistRoadModel = true;
                        return;
                    }
                });
            // 道モデルが見つからない
            if (bExistRoadModel == false) {
                ErrMsg = TEXT("3D都市モデルにLOD1道路モデルが含まれていません");
                return false;
            }
            return true;
        }

        static NavMeshBuildProgress CheckProgress(UWorld* World) {
            // まだ実行すらしていない
            const auto TwinLinkNavSys = ::FindFirstPersistentLevelActor<ATwinLinkNavSystem>(World);
            if (!TwinLinkNavSys)
                return  Error("");

            // モデルローダーが読み込まれていない
            TArray<AActor*> CityModelLoaders;
            UGameplayStatics::GetAllActorsOfClass(World, APLATEAUCityModelLoader::StaticClass(), CityModelLoaders);

            // モデルローダーがロード中
            for (const auto Actor : CityModelLoaders) {
                const auto CityModelLoader = Cast<APLATEAUCityModelLoader>(Actor);
                if (!CityModelLoader)
                    continue;
                if (CityModelLoader->Phase != ECityModelLoadingPhase::Finished)
                    return Error(TEXT("3D都市モデルのインポートがまだ完了していません"));
            }

            // モデルを読み込んでいない
            TArray<AActor*> CityModels;
            UGameplayStatics::GetAllActorsOfClass(World, APLATEAUInstancedCityModel::StaticClass(), CityModels);
            if (CityModels.IsEmpty())
                return Error(TEXT("3D都市モデルが読み込まれていません"));

            // CityModelの中に有効なモデルがあればOK
            // #NOTE : 背景モデルと違い, このレベルの開始地点となる建物は建物単体でインポートされるので道情報などはない
            {
                auto bExistValidCityModel = false;
                FString CityModelErrMsg;
                for (const auto Actor : CityModels) {
                    const auto CityModel = Cast<APLATEAUInstancedCityModel>(Actor);
                    FString ErrMsg;
                    bExistValidCityModel = bExistValidCityModel || CheckCityModel(CityModel, ErrMsg);
                    if (bExistValidCityModel)
                        break;
                    CityModelErrMsg = ErrMsg;
                }
                if (!bExistValidCityModel)
                    return Error(CityModelErrMsg);
            }
            // https://docs.unrealengine.com/5.2/en-US/API/Runtime/NavigationSystem/UNavigationSystemV1/
            UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
            if (!NavSys)
                return Error(TEXT("NavSystem not found"));

            // https://docs.unrealengine.com/5.0/ja/optimizing-navigation-mesh-generation-speed-in-unreal-engine/
            // ナビメッシュ生成中
            if (NavSys->IsNavigationBuildInProgress()) {
                // ...を描画するためのもの
                const auto TimeSec = World->GetTimeSeconds();
                const auto DotNum = ((int)TimeSec) % 3 + 1;
                FString ret = TEXT("ビルド中");
                ret.Append(FString::ChrN(DotNum, '.'));
                return Error(ret);
            }

            // ナビメッシュボリュームが作られていなければ実行されていない
            const auto Volume = ::FindFirstPersistentLevelActor<ANavMeshBoundsVolume>(World);
            if (!Volume)
                return Error("");

            // 成功
            if (NavSys->IsNavigationBuilt(World->GetWorldSettings()))
                return Complete();

            // その他エラー(ここには来ないはず)
            return Error(TEXT("Invalid Error"));
        }
    };

}


void UTwinLinkEditorNavSystem::MakeNavMesh(UEditorActorSubsystem* Editor, UWorld* World, UTwinLinkEditorNavSystemParam* Param) {

    checkf(Param != nullptr, TEXT("Paramがnullです"));
    if (!Param) {
        return;
    }

    checkf(Param->NavSystemBp != nullptr, TEXT("UTwinLinkEditorNavSystemParamのNavSystemBpがnullです"));
    if (!Param->NavSystemBp) {
        return;
    }

    // 既存のNavSystemは破壊して作り直す
    {
        TArray<AActor*> NavSystemActors;
        UGameplayStatics::GetAllActorsOfClass(World, ATwinLinkNavSystem::StaticClass(), NavSystemActors);
        for (auto& Actor : NavSystemActors)
            Actor->Destroy();
    }
    ATwinLinkNavSystem* NavSystem = Cast<ATwinLinkNavSystem>(Editor->SpawnActorFromObject(Param->NavSystemBp, FVector::Zero(), FRotator::ZeroRotator));
    NavSystem->SetActorLabel(TEXT("TwinLinkNavSystem"));
    const auto Result = ::ApplyNavMeshAffect(World, NavSystem);
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, ANavMeshBoundsVolume::StaticClass(), AllActors);

    if (Result.has_value() == false || Result->NavVolumeAabb.has_value() == false) {
        for (const auto Actor : AllActors)
            Actor->Destroy();
    }
    else {
        auto BbBox = Result->NavVolumeAabb;
        ANavMeshBoundsVolume* Volume = nullptr;
        if (AllActors.Num() > 0)
            Volume = Cast<ANavMeshBoundsVolume>(AllActors[0]);
        else
            Volume = Cast<ANavMeshBoundsVolume>(
                Editor->SpawnActorFromClass(ANavMeshBoundsVolume::StaticClass(), FVector::Zero(), FRotator::ZeroRotator));

        const auto Center = BbBox->GetCenter();
        const auto Extent = BbBox->GetExtent();
        const auto BrushBounds = Volume->Brush->Bounds;

        const FVector Scale = Extent / BrushBounds.BoxExtent;
        Volume->SetActorLocation(Center);
        Volume->SetActorScale3D(Scale);
        if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World)) {
            NavSys->OnNavigationBoundsUpdated(Volume);
            NavSys->Build();
        }
    }


}

void UTwinLinkEditorNavSystem::SetCanEverAffectNavigationAllActors(UWorld* World, bool bRelevant) {
    if (!World)
        return;

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
    for (const auto Actor : Actors) {
        ::SetCanEverAffectNavigationRecursively(Actor, bRelevant);
    }
}

FString UTwinLinkEditorNavSystem::GetNavMeshBuildingMessage(UWorld* World) {
    const auto Ret = ::NavMeshBuildProgress::CheckProgress(World);
    return Ret.Message;
}

bool UTwinLinkEditorNavSystem::IsNavMeshBuilt(UWorld* World) {
    const auto Ret = ::NavMeshBuildProgress::CheckProgress(World);
    return Ret.bIsBuildComplete;
}
