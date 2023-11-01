#include "NavSystem/TwinLinkNavSystem.h"

#include <array>
#include <numeric>

#include "NavigationSystem.h"
#include "PLATEAUInstancedCityModel.h"
#include "TwinLink.h"
#include "TwinLinkActorEx.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkPLATEAUCityModelEx.h"
#include "TwinLinkPLATEAUCityObjectGroupEx.h"
#include "TwinLinkWorldViewer.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h" 
#include "NavSystem/TwinLinkNavSystemEx.h"
#include "NavSystem/TwinLinkNavSystemPathFinder.h"

#include "libshape/datatype.h"
#include "libshape/shp_writer.h"

namespace {
    template<class T, class F>
    void ForeachChildActor(AActor* Self, F&& Func) {
        if (!Self)
            return;
        for (auto& Child : Self->Children) {
            if (auto Target = Cast<T>(Child))
                Func(Target);

            ForeachChildActor<T>(Child, std::forward<F>(Func));
        }
    }
    // 0, 1, 2, 3... -> 0, +1, -1, +2, -2と変換する
    int AlterSignSequence(int Index) {
        const auto Sign = ((Index & 1) << 1) - 1;
        const auto Offset = (Index + 1) >> 1;
        return Sign * Offset;
    }
}

ATwinLinkNavSystem::ATwinLinkNavSystem() {
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

ATwinLinkNavSystem* ATwinLinkNavSystem::GetInstance(const UWorld* World) {
    return TwinLinkActorEx::FindFirstActorInWorld<ATwinLinkNavSystem>(World);
}

ATwinLinkWorldViewer* ATwinLinkNavSystem::GetWorldViewer(const UWorld* World) {
    return TwinLinkActorEx::FindFirstActorInWorld<ATwinLinkWorldViewer>(World);
}

APlayerCameraManager* ATwinLinkNavSystem::GetPlayerCameraManager(const UWorld* World) {
    return TwinLinkActorEx::FindFirstActorInWorld<APlayerCameraManager>(World);
}

UTwinLinkFacilityInfoSystem* ATwinLinkNavSystem::GetFacilityInfoSystem(const UWorld* World) {
    if (!World)
        return nullptr;
    const auto GameInstance = World->GetGameInstance();
    if (!GameInstance)
        return nullptr;
    return GameInstance->GetSubsystem<UTwinLinkFacilityInfoSystem>();
}

UTwinLinkPeopleFlowSystem* ATwinLinkNavSystem::GetPeopleFlowSystem(const UWorld* World) {
    if (!World)
        return nullptr;
    const auto GameInstance = World->GetGameInstance();
    if (!GameInstance)
        return nullptr;
    return GameInstance->GetSubsystem<UTwinLinkPeopleFlowSystem>();
}

bool ATwinLinkNavSystem::FindNavMeshPoint(const UNavigationSystemV1* NavSys, const UStaticMeshComponent* StaticMeshComp, FVector& OutPos) {
    if (!StaticMeshComp || !NavSys)
        return false;
    auto Center = StaticMeshComp->Bounds.Origin;
    auto Extent = StaticMeshComp->Bounds.BoxExtent;

    // ナビメッシュの範囲内かどうか
    auto Pos = Center;
    Pos.Z = 0.f;
    const auto Size = Extent * 2;
    FNavLocation OutLocation;
    if (NavSys->ProjectPointToNavigation(Pos, OutLocation, Extent + FVector::One() * 1000)) {
        OutPos = OutLocation.Location;
        // #TODO : 適当な値
        OutPos.Z = Center.Z - Extent.Z;
        return true;
    }
    return false;
}

void ATwinLinkNavSystem::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    if (PathFindInfo.has_value()) {
        const auto bBeforeSuccess = PathFindInfo->IsSuccess();
        PathFindInfo->Update(GetWorld(), DemCollisionChannel, DemCollisionAabb.Min.Z, DemCollisionAabb.Max.Z, 10);
        const auto bAfterSuccess = PathFindInfo->IsSuccess();
        if (bBeforeSuccess == false && bAfterSuccess) {
            for (const auto Child : PathDrawers) {
                if (Child)
                    Child->DrawPath(PathFindInfo->HeightCheckedPoints);
            }
            OnReadyFindPathInfo(*PathFindInfo);
        }
    }

#ifdef WITH_EDITOR
    DebugDraw();
#endif
}

void ATwinLinkNavSystem::DebugDraw() {
#ifdef WITH_EDITOR
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
        return;

    if (DebugCallPathFinding) {
        DebugCallPathFinding = false;
        if (NowPathFinder) {
            FTwinLinkNavSystemFindPathInfo Tmp;
            if (NowPathFinder->RequestStartPathFinding(Tmp)) {
                PathFindInfo = Tmp;
            }
        }
    }

    if (DebugCallPeopleFlow) {
        DebugCallPeopleFlow = false;
        if (auto FlowSystem = GetPeopleFlowSystem(GetWorld())) {
            FlowSystem->Request(DebugPeopleFlowRequest);
        }
    }

    // NavSys->NavDataSet[0]->
#endif
}

void ATwinLinkNavSystem::DebugBeginPlay() {
#ifdef WITH_EDITOR
#endif
}


void ATwinLinkNavSystem::BeginPlay() {
    Super::BeginPlay();

    if (auto* Viewer = GetWorldViewer(GetWorld())) {
        Viewer->EvOnClickedFacility.AddUObject(this, &ATwinLinkNavSystem::OnFacilityClick);
    }

    // #TODO : FacilityInfoSystem::FindFacilityを使った方がよさそうだが毎回全検索しているので自前でキャッシュ作るようにする
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APLATEAUInstancedCityModel::StaticClass(), AllActors);

    std::optional<FBox> Fields = std::nullopt;
    for (const auto Actor : AllActors) {
        const auto InstanceCityModel = Cast<APLATEAUInstancedCityModel>(Actor);
        if (!InstanceCityModel)
            continue;
        for (auto [CityObjectGroup, MeshType, LodLevel] : TwinLinkPLATEAUInstancedCityModelScanner(InstanceCityModel)) {
            if (MeshType == FTwinLinkFindCityModelMeshType::Bldg) {
                BuildingMap.Add(CityObjectGroup->GetName(), FTwinLinkNavSystemBuildingInfo(CityObjectGroup));
            }

            if (MeshType != FTwinLinkFindCityModelMeshType::Tran) {
                const auto Bb = FTwinLinkPLATEAUCityObjectGroupEx::GetBoundingBox(CityObjectGroup.Get());
                if (Bb.has_value()) {
                    if (Fields.has_value() == false)
                        Fields = *Bb;
                    else
                        Fields = *Fields + *Bb;
                }
            }
        }
    }
    // 
    FieldBb = Fields.value_or(FBox(FVector::Zero(), FVector::Zero()));

    // FacilityInfoを設定する
    if (const auto FacilitySystem = GetFacilityInfoSystem(GetWorld())) {
        for (auto& Item : FacilitySystem->GetFacilityInfoCollectionAsMap()) {
            auto FeatureId = Item.Value->GetFeatureID();
            if (BuildingMap.Contains(FeatureId))
                BuildingMap[FeatureId].FacilityInfo = Item.Value;
        }
    }

    // #TODO : 一時的にナビメッシュポイントは実行時に適当に設定する
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys) {
        if (const auto FacilitySystem = GetFacilityInfoSystem(GetWorld())) {
            for (auto& Item : FacilitySystem->GetFacilityInfoCollectionAsMap()) {
                const auto FeatureId = Item.Value->GetFeatureID();
                if (BuildingMap.Contains(FeatureId)) {
                    FVector OutPos;
                    if (FindNavMeshPoint(NavSys, BuildingMap[FeatureId].CityObjectGroup.Get(), OutPos)) {
                        Item.Value->SetEntrances(TArray<FVector>{OutPos});
                    }

                }
            }
        }
    }

#ifdef WITH_EDITOR
    DebugBeginPlay();
#endif
}

void ATwinLinkNavSystem::ChangeMode(NavSystemMode Mode, bool bForce) {
    if (NowSelectedMode == Mode && !bForce)
        return;

    // 既存のDrawerクラスなどを削除する
    Clear();

    NowSelectedMode = Mode;
    if (NavSystemModeT(NowSelectedMode).IsValid() == false)
        return;

    if (!RuntimeParam) {
        UKismetSystemLibrary::PrintString(this, "NavSystem RuntimeParam != nullptr");
        return;
    }
    // パス描画クラスを再生成する
    TwinLinkActorEx::SpawnChildActor(this, RuntimeParam->PathDrawerBp, TEXT("PathDrawerActor"));
    ForeachChildActor<AUTwinLinkNavSystemPathDrawer>(this, [&](AUTwinLinkNavSystemPathDrawer* Child) {
        PathDrawers.Add(Child);
        });

    if (RuntimeParam->PathFinderBp.Contains(Mode)) {
        FString Name = TEXT("PathFinder");
        Name.AppendInt(static_cast<int>(Mode));
        NowPathFinder = TwinLinkActorEx::SpawnChildActor(this, RuntimeParam->PathFinderBp[Mode], TCHAR_TO_WCHAR(*Name));
        NowPathFinder->OnReadyPathFinding.AddUObject(this, &ATwinLinkNavSystem::OnReadyPathFinding);
    }
}

bool ATwinLinkNavSystem::TryGetReadyFindPathInfo(FTwinLinkNavSystemFindPathInfo& Out) const {
    if (PathFindInfo.has_value()) {
        Out = *PathFindInfo;
        return true;
    }
    return false;
}

const UTwinLinkNavSystemParam* ATwinLinkNavSystem::GetRuntimeParam() const {
    return RuntimeParam;
}

FTwinLinkNavSystemFindPathUiInfo ATwinLinkNavSystem::GetDrawMoveTimeUiInfo(TwinLinkNavSystemMoveType MoveType, const FBox2D& ScreenRange) const {
    if (PathFindInfo.has_value() == false || PathFindInfo->IsSuccess() == false)
        return FTwinLinkNavSystemFindPathUiInfo();
    if (TwinLinkNavSystemMoveTypeT(MoveType).IsValid() == false)
        return FTwinLinkNavSystemFindPathUiInfo();

    const APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    auto& HeightCheckedPoints = PathFindInfo->HeightCheckedPoints;

    // 総距離
    const auto Length = PathFindInfo->GetTotalLength();

    const auto Meter = Length / RuntimeParam->WorldUnitMeter;
    const auto Sec = Meter * 3.6f / RuntimeParam->GetMoveSpeedKmPerH(MoveType);

    const auto Center = FMath::Max(0, HeightCheckedPoints.Num() - 1) >> 1;

    FVector2D NearestPos;
    float NearestSqrLen = -1;
    for (auto I = 0; I < HeightCheckedPoints.Num(); ++I) {
        const auto Index = Center + AlterSignSequence(I);
        FVector2D Tmp;
        if (UGameplayStatics::ProjectWorldToScreen(Controller, HeightCheckedPoints[Index], Tmp)) {
            if (ScreenRange.IsInsideOrOn(Tmp)) {
                return FTwinLinkNavSystemFindPathUiInfo(Meter, Sec, Tmp);
            }
            const auto SqrLen = (ScreenRange.GetCenter() - Tmp).SquaredLength();
            if (NearestSqrLen < 0.f || SqrLen < NearestSqrLen) {
                NearestSqrLen = SqrLen;
                NearestPos = Tmp;
            }
        }
    }
    if (NearestSqrLen >= 0.f)
        return FTwinLinkNavSystemFindPathUiInfo(Meter, Sec, NearestPos);
    return FTwinLinkNavSystemFindPathUiInfo();
}

bool ATwinLinkNavSystem::GetOutputPathInfo(FTwinLinkNavSystemOutputPathInfo& Out) const {
    if (PathFindInfo.has_value() == false || PathFindInfo->IsSuccess() == false)
        return false;

    const auto& TwinLinkModule = FTwinLinkModule::Get();
    const auto CityModel = Cast<APLATEAUInstancedCityModel>(TwinLinkModule.GetCityModel());
    if (!CityModel) {
        UE_LOG(LogTemp, Error, TEXT("TwinLinkModuleにCityModelが設定されていません"));
        return false;
    }
    auto& Geo = CityModel->GeoReference.GetData();
    FTwinLinkNavSystemOutputPathInfo Ret;


    const auto& Points = PathFindInfo->HeightCheckedPoints;
    const auto& Indices = PathFindInfo->PathPointsIndices;
    auto ToGeoCoord = [&](int I) {
        const auto V = Points[Indices[I]];
        return TVec3d(V.X, V.Y, V.Z);
    };
    Ret.Start = Geo.unproject(ToGeoCoord(0));
    Ret.End = Geo.unproject(ToGeoCoord(Indices.Num() - 1));
    for (auto i = 1; i < Indices.Num() - 1; ++i)
        Ret.Route.Add(Geo.unproject(ToGeoCoord(i)));

    // 総距離
    const auto Length = PathFindInfo->GetTotalLength();
    const auto Meter = Length / RuntimeParam->WorldUnitMeter;
    Ret.MinutesByWalk = Meter * 3.6f / 60.f / RuntimeParam->GetMoveSpeedKmPerH(TwinLinkNavSystemMoveType::Walk);
    Ret.MinutesByCar = Meter * 3.6f / 60.f / RuntimeParam->GetMoveSpeedKmPerH(TwinLinkNavSystemMoveType::Car);

    Out = Ret;
    return true;
}

bool ATwinLinkNavSystem::ExportOutputPathInfo(const FString& Path) const {
    FTwinLinkNavSystemOutputPathInfo Out;
    if (GetOutputPathInfo(Out) == false)
        return false;

    // Shape layerに変換
    shp::Layer layer;
    layer.type = shp::ShapeType::PolyLineZ;

    layer.attribute_database.fields = { "MIN_CAR", "MIN_WALK" };

    std::vector<std::string> attributes;
    attributes.push_back(std::to_string(Out.MinutesByCar));
    attributes.push_back(std::to_string(Out.MinutesByWalk));
    layer.attribute_database.attributes = { attributes };

    shp::Shape shape;
    for (const auto Point : Out.Route)
    {
        shape.points.emplace_back(Point.longitude, Point.latitude, Point.height);
    }
    layer.shapes.push_back(std::move(shape));

    // Shapefile書き出し
    shp::ShpWriter::write(TCHAR_TO_UTF8(*Path), layer);

    // エクスプローラーで書き出し先のフォルダを開く
    FPlatformProcess::LaunchFileInDefaultExternalApplication(*FPaths::GetPath(Path), nullptr, ELaunchVerb::Open);


    return true;
}

ATwinLinkNavSystemPathFinder* ATwinLinkNavSystem::GetNowPathFinder() {
    return NowPathFinder;
}

FTwinLinkNavSystemBuildingInfo ATwinLinkNavSystem::GetBaseBuilding() const {
    // #TODO : 一時的に管理対象物の情報も仮で設定する
    const auto& TwinLinkModule = FTwinLinkModule::Get();

    // 管理対象物
    const auto TargetBuilding = TwinLinkModule.GetFacilityModelBuildingComponent();
    if (TargetBuilding.IsValid() == false)
        return FTwinLinkNavSystemBuildingInfo();

    if (BuildingMap.Contains(TargetBuilding->GetName()))
        return BuildingMap[TargetBuilding->GetName()];

    return FTwinLinkNavSystemBuildingInfo();
}

TArray<FTwinLinkNavSystemBuildingInfo> ATwinLinkNavSystem::GetBuildingInfos() {

    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    TArray<FTwinLinkNavSystemBuildingInfo> Ret;
    const auto GameInstance = GetWorld()->GetGameInstance();
    auto BaseBuilding = GetBaseBuilding();
    if (const auto FacilityInfo = GameInstance->GetSubsystem<UTwinLinkFacilityInfoSystem>()) {
        for (auto Item : FacilityInfo->GetFacilityInfoCollectionAsMap()) {
            if (!Item.Value.Get())
                continue;
            if (Item.Value->GetEntrances().IsEmpty()) {
                //FindNavMeshPoint(NavSys, Item.Value->)
            }

            // 管理対象物は除く
            if (BaseBuilding.CityObjectGroup.IsValid() && Item.Value->GetFeatureID() == BaseBuilding.CityObjectGroup->GetName())
                continue;

            if (BuildingMap.Contains(Item.Value->GetFeatureID())) {
                auto& Info = BuildingMap[Item.Value->GetFeatureID()];
                // FacilityInfoが更新されていれば設定(新規で追加された建物など)
                if (Info.FacilityInfo != Item.Value)
                    Info.FacilityInfo = Item.Value;
                Ret.Add(Info);
            }
        }
    }
    return Ret;
}

const FTwinLinkNavSystemBuildingInfo* ATwinLinkNavSystem::FindBuildingInfo(
    TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup) const {
    if (CityObjectGroup.IsValid() == false)
        return nullptr;

    const auto Key = CityObjectGroup->GetName();
    if (!BuildingMap.Contains(Key))
        return nullptr;
    return &BuildingMap[Key];
}

void ATwinLinkNavSystem::OnFacilityClick(FHitResult Info) const {
    const auto CityObject = Cast<UPLATEAUCityObjectGroup>(Info.Component);
    if (!CityObject) {
        UKismetSystemLibrary::PrintString(this, "Invalid CityObject");
        return;
    }
    const auto Key = CityObject->GetName();
    if (!BuildingMap.Contains(Key))
        return;

    // 管理対象の建物の場合は除く
    const auto BaseBuilding = GetBaseBuilding();
    if (BaseBuilding.CityObjectGroup == CityObject)
        return;

    const auto& BuildingInfo = BuildingMap[Key];
    if (BuildingInfo.FacilityInfo.IsValid()) {
        OnFacilityClicked.Broadcast(Info, BuildingInfo);
    }

    if (const auto CityModel = Cast<APLATEAUInstancedCityModel>(CityObject->GetAttachParentActor())) {
        const TVec3d Point(Info.Location.X, Info.Location.Y, Info.Location.Z);
        const auto Geo = CityModel->GeoReference.GetData().unproject(Point);
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Lat : %f, Lon : %f"), Geo.latitude, Geo.longitude));
    }
}
void ATwinLinkNavSystem::Clear() {
    if (NowPathFinder) {
        NowPathFinder->Clear();
        NowPathFinder->Destroy();
    }
    NowPathFinder = nullptr;
    // 既存のパス描画クラスを削除
    for (const auto Drawer : PathDrawers)
        Drawer->Destroy();
    PathDrawers.RemoveAll([](auto& a) { return true; });
    PathFindInfo = std::nullopt;
    NowSelectedMode = NavSystemMode::Undefined;
}

bool ATwinLinkNavSystem::IsValid() const {
    return UTwinLinkNavSystemEx::IsValidBuilding(GetBaseBuilding()) && GetRuntimeParam() != nullptr;
}

void ATwinLinkNavSystem::OnReadyPathFinding() {
    if (NowPathFinder) {
        FTwinLinkNavSystemFindPathInfo Tmp;
        if (NowPathFinder->RequestStartPathFinding(Tmp)) {
            PathFindInfo = Tmp;
        }
    }
}
