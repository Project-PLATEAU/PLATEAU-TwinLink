// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkRoomSelectSystem.h"

#include "PLATEAUCityObjectGroup.h"
#include "ProceduralMeshComponent.h"
#include "TwinLinkCommon.h"
#include "TwinLinkWorldViewer.h"
#include "Misc/TwinLinkActorEx.h"
#include "PhysicsEngine/BodySetup.h"

namespace {
    /**
     * @brief レイキャストヒットしたメッシュ内で指定UVと同じUVを持つエッジを重複なしで全て取得
     */
    void GetMeshInfoByUV(UStaticMeshComponent* StaticMeshComponent, const FVector2D& UV, TArray<FVector>& OutPositions, TArray<int>& OutTriangles, TArray<FVector2D>& OutUVs, const int32 UVChannel = 3, const int32 LodIndex = 0) {
        const auto& BodySetup = StaticMeshComponent->GetBodySetup();
        if (BodySetup == nullptr) {
            UE_LOG(LogTemp, Error, TEXT("BodySetup == nullptr"));
            return;
        }

        const FTransform& Transform = StaticMeshComponent->GetAttachParentActor()->GetActorTransform();

        const auto& StaticMesh = StaticMeshComponent->GetStaticMesh();
        const auto& [IndexBuffer, VertPositions, VertUVs] = BodySetup->UVInfo;

        int IndexOffset = 0;

        for (int32 FaceIndex = 0; FaceIndex < StaticMesh->GetNumTriangles(LodIndex); ++FaceIndex) {
            if (VertUVs.IsValidIndex(UVChannel) && IndexBuffer.IsValidIndex(FaceIndex * 3 + 2)) {
                const int32 Index0 = IndexBuffer[FaceIndex * 3 + 0];
                const int32 Index1 = IndexBuffer[FaceIndex * 3 + 1];
                const int32 Index2 = IndexBuffer[FaceIndex * 3 + 2];

                FVector Pos0 = Transform.TransformPosition(VertPositions[Index0]);
                FVector Pos1 = Transform.TransformPosition(VertPositions[Index1]);
                FVector Pos2 = Transform.TransformPosition(VertPositions[Index2]);

                FVector2D UV0 = VertUVs[UVChannel][Index0];
                FVector2D UV1 = VertUVs[UVChannel][Index1];
                FVector2D UV2 = VertUVs[UVChannel][Index2];

                // Find barycentric coords
                // 第一引数に自身の頂点を与えることで必ず同じBaryCoordsが得られるようにしている（FaceIndexによってのみUVが変化する）
                const FVector BaryCoords = FMath::ComputeBaryCentric2D(Pos0, Pos0, Pos1, Pos2);
                // Use to blend UVs
                const auto& TargetUV = BaryCoords.X * UV0 + BaryCoords.Y * UV1 + BaryCoords.Z * UV2;
                // 同じUVを持つ頂点取得
                if (static_cast<int32>(TargetUV.X) == static_cast<int32>(UV.X) && static_cast<int32>(TargetUV.Y) == static_cast<int32>(UV.Y)) {
                    OutPositions.Add(Pos0);
                    OutUVs.Add(UV0);

                    OutTriangles.Add(IndexOffset++);
                    OutPositions.Add(Pos1);
                    OutUVs.Add(UV1);
                    OutTriangles.Add(IndexOffset++);
                    OutPositions.Add(Pos2);
                    OutUVs.Add(UV2);
                    OutTriangles.Add(IndexOffset++);
                }
            }
        }
    }
}

UTwinLinkRoomSelectSystem::UTwinLinkRoomSelectSystem() {
}

UTwinLinkRoomSelectSystem::~UTwinLinkRoomSelectSystem() {
}

void UTwinLinkRoomSelectSystem::UpdateSelection(const FHitResult& HitResult) {
    const auto& HitPrimitiveComponent = HitResult.Component.Get();
    if (HitPrimitiveComponent == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Error, TEXT("HitPrimitiveComponent == nullptr"));
        return;
    }

    const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(HitPrimitiveComponent);
    if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Error, TEXT("StaticMeshComponent == nullptr or StaticMeshComponent->GetStaticMesh() == nullptr"));
        return;
    }

    FVector2D UV;
    UPLATEAUCityObjectGroup::FindCollisionUV(HitResult, UV);
    UpdateSelectionByUV(StaticMeshComponent, UV);
}

void UTwinLinkRoomSelectSystem::UpdateSelectionByUV(UStaticMeshComponent* StaticMeshComponent, const FVector2D& UV) {
    SelectedMesh = MakeWeakObjectPtr(StaticMeshComponent);
    SelectedUV = UV;

    SelectedMeshPositions.Empty();
    SelectedMeshTriangles.Empty();
    SelectedMeshUVs.Empty();
    GetMeshInfoByUV(SelectedMesh.Get(), SelectedUV, SelectedMeshPositions, SelectedMeshTriangles, SelectedMeshUVs);

    auto CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(StaticMeshComponent);
    if (CityObjectGroup == nullptr) {
        UE_TWINLINK_LOG(LogTemp, Error, TEXT("Selected mesh is not CityObjectGroup"));
        return;
    }

    RoomID = CityObjectGroup->GetCityObjectByUV(UV).GmlID;

    Focus();

    SelectRoomEvent.Broadcast();
}

void UTwinLinkRoomSelectSystem::Focus()
{
    if (SelectedMesh == nullptr)
        return;

    const auto WorldViewer = TwinLinkActorEx::FindFirstActorInWorld<ATwinLinkWorldViewer>(GetWorld());
    const auto Center = GetSelectedMeshCenter();
    const auto Distance = FMath::Abs(Center.Z + 3000.0f - Center.Z);
    const auto Dir = FVector(1.0f, 0.0f, 0.0f).GetSafeNormal();

    FVector V0 = Center;
    FVector V1 = V0 + (-Dir * Distance);
    FVector V2 = FVector(V0.X, V0.Y, V0.Z + Distance);

    //対象からのベクトル
    const auto V01 = V1 - V0;
    const auto V02 = V2 - V0;

    //軸を求める
    auto RotationAxis = FVector::CrossProduct(V01, V02).GetSafeNormal();

    //回転
    const auto V2Primed = FQuat(RotationAxis, FMath::DegreesToRadians(60.0f)) * V01;

    //位置を求める
    const auto v3PrimedFinal = V0 + V2Primed;

    //WorldViewerを移動（対象に向く）
    WorldViewer->SetLocationLookAt(v3PrimedFinal, V0, 0.5f);
}

FString UTwinLinkRoomSelectSystem::GetSelectedRoomID() const {
    return RoomID;
}

FVector2D UTwinLinkRoomSelectSystem::FindCollisionUV(const FHitResult& HitResult) {
    FVector2D UV;
    UPLATEAUCityObjectGroup::FindCollisionUV(HitResult, UV);
    return UV;
}
