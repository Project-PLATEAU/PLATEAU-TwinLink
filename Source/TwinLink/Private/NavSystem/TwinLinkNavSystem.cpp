#include "NavSystem/TwinLinkNavSystem.h"

#include <array>
#include <numeric>

#include "NavigationSystem.h"
#include "PLATEAUInstancedCityModel.h"
#include "TwinLink.h"
#include "TwinLinkActorEx.h"
#include "TwinLinkFacilityInfo.h"
#include "TwinLinkFacilityInfoSystem.h"
#include "TwinLinkMathEx.h"
#include "TwinLinkPLATEAUCityModelEx.h"
#include "TwinLinkPLATEAUCityObjectGroupEx.h"
#include "TwinLinkWorldViewer.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h" 
#include "Engine/Canvas.h"
#include "NavSystem/TwinLinkNavSystemEx.h"
#include "NavSystem/TwinLinkNavSystemPathFinder.h"

#include "libshape/datatype.h"
#include "libshape/shp_writer.h"
#include "DrawDebugHelpers.h"
#include "TwinLinkPlayerController.h"
#include "GameFramework/HUD.h"

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
    // ハイトマップのセルサイズ
    constexpr float DEM_CELL_SIZE = 100;

    TMap<NavSystemPathPointType, FBox2D> DebugPathPointScreenRect;
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

ATwinLinkNavSystemEntranceLocator* ATwinLinkNavSystem::GetEntranceLocator(const UWorld* World) {
    if (const auto NavSys = GetInstance(World))
        return NavSys->GetEntranceLocator();
    return nullptr;
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
                    Child->DrawPath(PathFindInfo->HeightCheckedPoints, DeltaSeconds);
            }
            OnReadyFindPathInfo(*PathFindInfo);
        }
    }

    if (EntranceLocator)
        EntranceLocator->SetActorHiddenInGame(FTwinLinkEntranceLocatorNode::IsAnyNodeVisible() == false);
#if WITH_EDITOR
    DebugDraw();
#endif
}

bool ATwinLinkNavSystem::IsEntranceLocatorVisible() const {
    return EntranceLocator && FTwinLinkEntranceLocatorNode::IsAnyNodeVisible();
}

void ATwinLinkNavSystem::BuildDemHeightMap() {
    auto MinX = FMath::FloorToInt(DemCollisionAabb.Min.X / DEM_CELL_SIZE);
    auto MaxX = FMath::CeilToInt(DemCollisionAabb.Max.X / DEM_CELL_SIZE);
    auto MinY = FMath::FloorToInt(DemCollisionAabb.Min.Y / DEM_CELL_SIZE);
    auto MaxY = FMath::CeilToInt64(DemCollisionAabb.Max.Y / DEM_CELL_SIZE);

    DemHeightMapSizeX = MaxX - MinX + 1;
    DemHeightMapSizeY = MaxY - MinY + 1;
    DemHeightMap.Init(DemCollisionAabb.Min.Z - 100, DemHeightMapSizeX * DemHeightMapSizeY);
    for (auto X = MinX; X <= MaxX; ++X) {
        for (auto Y = MinY; Y <= MaxY; ++Y) {
            auto P = FVector3d(X * DEM_CELL_SIZE, Y * DEM_CELL_SIZE, 0);
            auto Start = FVector3d(P.X, P.Y, DemCollisionAabb.Max.Z + 1);
            auto End = FVector3d(P.X, P.Y, DemCollisionAabb.Min.Z - 1);
            FHitResult HeightResult;

            if (GetWorld()->LineTraceSingleByChannel(HeightResult, Start, End, DemCollisionChannel)) {
                auto Index = PositionToDemHeightMapIndex(Start);
                if(Index >= 0 && Index < DemHeightMap.Num())
                    DemHeightMap[Index] = HeightResult.Location.Z;
            }
        }
    }
}

void ATwinLinkNavSystem::DebugDraw() {
#if WITH_EDITOR
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

    if (DebugDrawDemHeightMap) {


        const APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        auto MousePos = ATwinLinkPlayerController::GetMousePosition(Controller);
        if (MousePos.has_value())
        {
            auto Ray = ATwinLinkPlayerController::ScreenToWorldRayThroughBoundingBox(Controller, *MousePos, DemCollisionAabb, 100);
            if(Ray.has_value())
            {
                FHitResult HitResult;
                if (GetWorld()->LineTraceSingleByChannel(HitResult, Ray->Min, Ray->Max, ECC_WorldStatic)) {
                    auto Center = HitResult.Location;
                    auto Extent = DEM_CELL_SIZE * 50;
                    FBox Range(Center - Extent, Center + Extent);
                    for(auto X = Range.Min.X; X < Range.Max.X; X += DEM_CELL_SIZE)
                    {
                        for(auto Y = Range.Min.Y; Y < Range.Max.Y; Y += DEM_CELL_SIZE)
                        {
                            auto Z = GetDemHeight(FVector2D(X, Y));
                            if (Z.has_value() == false)
                                continue;
                            const auto C = FVector(X, Y, *Z);
                            const auto E = FVector3d(DEM_CELL_SIZE, DEM_CELL_SIZE, 10);
                            DrawDebugBox(GetWorld(), C, E, FColor::Red);
                        }
                    }
                }
            }
        }
       
    }

    {
        const APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        auto GetMarkerScreenBb = [&](NavSystemPathPointType PointType) -> std::optional<FBox2D> {
            if (!NowPathFinder)
                return std::nullopt;
            auto Bb = NowPathFinder->GetPathLocationBox(PointType);
            if (Bb.has_value() == false)
                return std::nullopt;
            FVector Vertices[8];
            Bb->GetVertices(Vertices);
            std::optional<FBox2D> Ret;
            for (auto& V : Vertices) {
                FVector2D Tmp;
                if (UGameplayStatics::ProjectWorldToScreen(Controller, V, Tmp)) {
                    if (Ret.has_value() == false) {
                        Ret = FBox2D(Tmp, Tmp);
                    }
                    else {
                        Ret = *Ret + FBox2D(Tmp, Tmp);
                    }
                }
            }
            return Ret;
        };

        auto DrawRect = [&](NavSystemPathPointType Type, FColor Color) {
            auto Bb = GetMarkerScreenBb(Type);
            if (Bb.has_value() == false)
                return;
            auto& Box = *Bb;;
            auto HUD = Controller->GetHUD();

            //HUD->DrawRDeecDet(Color, Box.Min.X, Box.Min.Y, Box.GetSize().X, Box.GetSize().Y);
        };

        DrawRect(NavSystemPathPointType::Start, FColor::Red);
        DrawRect(NavSystemPathPointType::Dest, FColor::Blue);
    }
    // NavSys->NavDataSet[0]->
#endif
}

void ATwinLinkNavSystem::DebugBeginPlay() {
#if WITH_EDITOR
#endif
}

std::optional<double> ATwinLinkNavSystem::GetDemHeight(const FVector2D& Pos) const {
    const auto Offset = Pos - TwinLinkMathEx::XY(DemCollisionAabb.Min);

    // バイライナー補間
    const auto X = FMath::Clamp(Offset.X / DEM_CELL_SIZE, 0.f, 1.f * DemHeightMapSizeX);
    const auto Y = FMath::Clamp(Offset.Y / DEM_CELL_SIZE, 0.f, 1.f * DemHeightMapSizeY);

    const auto X1 = FMath::FloorToInt(X);
    const auto Y1 = FMath::FloorToInt(Y);
    const auto X2 = FMath::CeilToInt(X);
    const auto Y2 = FMath::CeilToInt(Y);

    const auto Xt = 1.f - (X - X1);
    const auto Yt = 1.f - (Y - Y1);

    // 範囲チェックしているから値が入っていることは保証されている
    const auto P11 = *GetDemHeight(X1, Y1);
    const auto P12 = *GetDemHeight(X1, Y2);
    const auto P22 = *GetDemHeight(X2, Y2);
    const auto P21 = *GetDemHeight(X2, Y1);

    const auto Px1 = FMath::Lerp(P11, P21, Xt);
    const auto Px2 = FMath::Lerp(P12, P22, Xt);

    return FMath::Lerp(Px1, Px2, Yt);
}

std::optional<double> ATwinLinkNavSystem::GetDemHeight(int Index) const {
    if (Index < 0 || Index >= DemHeightMap.Num())
        return std::nullopt;

    // 設定値がAABBの範囲外ならそこには道は存在しない
    const auto Height = DemHeightMap[Index];
    if (Height < DemCollisionAabb.Min.Z)
        return std::nullopt;
    return Height;
}

std::optional<double> ATwinLinkNavSystem::GetDemHeight(int CellX, int CellY) const {
    return GetDemHeight(DemHeightMapCellToIndex(CellX, CellY));
}

int ATwinLinkNavSystem::PositionToDemHeightMapIndex(const FVector3d& Pos) const {
    auto Offset = Pos - DemCollisionAabb.Min;
    if (Offset.X < 0 || Offset.Z < 0)
        return -1;

    auto X = FMath::FloorToInt(Offset.X / DEM_CELL_SIZE);
    auto Y = FMath::FloorToInt(Offset.Y / DEM_CELL_SIZE);
    return DemHeightMapCellToIndex(X, Y);
}

std::optional<FVector3d> ATwinLinkNavSystem::DemHeightMapIndexToPosition(int Index) const {
    int X;
    int Y;
    if (TryDemHeightMapIndexToCell(Index, X, Y) == false)
        return std::nullopt;

    return FVector3d(1.f * X * DEM_CELL_SIZE, 1.f * Y * DEM_CELL_SIZE, 0.f) + DemCollisionAabb.Min;
}

int ATwinLinkNavSystem::DemHeightMapCellToIndex(int X, int Y) const {
    return Y * DemHeightMapSizeX + X;
}

bool ATwinLinkNavSystem::TryDemHeightMapIndexToCell(int Index, int& OutX, int& OutY) const {
    if (Index < 0 || Index >= DemHeightMap.Num())
        return false;

    OutX = Index % DemHeightMapSizeX;
    OutY = Index / DemHeightMapSizeX;
    return true;
}

void ATwinLinkNavSystem::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) {
    Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
#ifdef WITH_EDITOR
    const APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    auto GetMarkerScreenBb = [&](NavSystemPathPointType PointType) -> std::optional<FBox2D> {
        if (!NowPathFinder)
            return std::nullopt;
        auto Bb = NowPathFinder->GetPathLocationBox(PointType);
        if (Bb.has_value() == false)
            return std::nullopt;
        FVector Vertices[8];
        Bb->GetVertices(Vertices);
        std::optional<FBox2D> Ret;
        for (auto& V : Vertices) {
            FVector2D Tmp;
            if (UGameplayStatics::ProjectWorldToScreen(Controller, V, Tmp)) {
                if (Ret.has_value() == false) {
                    Ret = FBox2D(Tmp, Tmp);
                }
                else {
                    Ret = *Ret + FBox2D(Tmp, Tmp);
                }
            }
        }
        return Ret;
    };
    auto DrawRect = [&](NavSystemPathPointType Type, FColor Color) {
        auto Bb = GetMarkerScreenBb(Type);
        if (Bb.has_value() == false)
            return;
        auto& Box = *Bb;;
        Canvas->K2_DrawLine(Box.Min, Box.Max, 1, Color);
        Canvas->K2_DrawBox(Box.Min, Box.GetSize(), 1, Color);
    };

    DrawRect(NavSystemPathPointType::Start, FColor::Red);
    DrawRect(NavSystemPathPointType::Dest, FColor::Blue);
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

    // 管理画面で使う入り口設定用のアクターを生成
    EntranceLocator = TwinLinkActorEx::SpawnChildActor(this, EntranceLocatorBp, TEXT("EntranceLocator"));
    EntranceLocator->SetActorHiddenInGame(true);

    // #TODO : 一時的にナビメッシュポイントは実行時に適当に設定する
    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys) {
        if (const auto FacilitySystem = GetFacilityInfoSystem(GetWorld())) {
            for (auto& Item : FacilitySystem->GetFacilityInfoCollectionAsMap()) {
                if (Item.Value->GetEntrances().IsEmpty() == false)
                    continue;
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

#if WITH_EDITOR
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
    for(auto& Bp : RuntimeParam->PathDrawerBps)
    {
        check(Bp);
        if (Bp.Get() == nullptr)
            continue;
        TwinLinkActorEx::SpawnChildActor(this, Bp, Bp->GetName());
        ForeachChildActor<AUTwinLinkNavSystemPathDrawer>(this, [&](AUTwinLinkNavSystemPathDrawer* Child) {
            PathDrawers.Add(Child);
            });
    }

    if (RuntimeParam->PathFinderBp.Contains(Mode)) {
        FString Name = TEXT("PathFinder");
        Name.AppendInt(static_cast<int>(Mode));
        NowPathFinder = TwinLinkActorEx::SpawnChildActor(this, RuntimeParam->PathFinderBp[Mode], Name);
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

FTwinLinkNavSystemFindPathUiInfo ATwinLinkNavSystem::GetDrawMoveTimeUiInfo(TwinLinkNavSystemMoveType MoveType, const FBox2D& ScreenRange, const FVector2D& UiPanelSize) const {
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

    // スクリーン上のLeftTop
    FVector2D NearestPos;
    float NearestSqrLen = -1;
    for (auto I = 0; I < HeightCheckedPoints.Num(); ++I) {
        const auto Index = Center + AlterSignSequence(I);
        FVector2D Tmp;
        if (UGameplayStatics::ProjectWorldToScreen(Controller, HeightCheckedPoints[Index], Tmp)) {
            const auto SqrLen = (ScreenRange.GetCenter() - Tmp).SquaredLength();
            if (ScreenRange.IsInsideOrOn(Tmp)) {
                NearestPos = Tmp;
                NearestSqrLen = SqrLen;
                break;
            }
            if (NearestSqrLen < 0.f || SqrLen < NearestSqrLen) {
                NearestSqrLen = SqrLen;
                NearestPos = Tmp;
            }
        }
    }

    if (NearestSqrLen >= 0.f) {
        auto GetMarkerScreenBb = [&](NavSystemPathPointType PointType) -> std::optional<FBox2D> {
            if (!NowPathFinder)
                return std::nullopt;
            auto Bb = NowPathFinder->GetPathLocationBox(PointType);
            if (Bb.has_value() == false)
                return std::nullopt;
            FVector Vertices[8];
            Bb->GetVertices(Vertices);
            std::optional<FBox2D> Ret;
            for (auto& V : Vertices) {
                FVector2D Tmp;
                if (UGameplayStatics::ProjectWorldToScreen(Controller, V, Tmp)) {
                    if (Ret.has_value() == false) {
                        Ret = FBox2D(Tmp, Tmp);
                    }
                    else {
                        Ret = *Ret + FBox2D(Tmp, Tmp);
                    }
                }
            }
            return Ret;
        };
        auto PanelBox = FBox2D(NearestPos, NearestPos + UiPanelSize);
        const auto StartScreenBox = GetMarkerScreenBb(NavSystemPathPointType::Start);
        const auto DestScreenBox = GetMarkerScreenBb(NavSystemPathPointType::Dest);
        TArray<FBox2D> MarkerBoxes;
        if (StartScreenBox.has_value())
            MarkerBoxes.Add(*StartScreenBox);

        if (DestScreenBox.has_value())
            MarkerBoxes.Add(*DestScreenBox);

        if (MarkerBoxes.Num() == 1) {
            auto& B = MarkerBoxes[0];
            if (B.Intersect(PanelBox)) {
                auto Rect = FBox2D(B.Min - UiPanelSize, B.Max - UiPanelSize);
                NearestPos = TwinLinkMathEx::GetClosestPointOnEdge(Rect, NearestPos);
            }
        }
        else if (MarkerBoxes.Num() == 2) {

            auto B = TwinLinkMathEx::MoveBox2DToNonOverlapPoint(FBox2D(NearestPos, NearestPos + UiPanelSize), MarkerBoxes[0], MarkerBoxes[1], ScreenRange);
            NearestPos = B.Min;
        }


        return FTwinLinkNavSystemFindPathUiInfo(Meter, Sec, NearestPos);
    }
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
    for (const auto Point : Out.Route) {
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
