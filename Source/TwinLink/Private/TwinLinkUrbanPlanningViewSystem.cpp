// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkUrbanPlanningViewSystem.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "TwinLink.h"
#include "TwinLinkCommon.h"
#include "PLATEAUInstancedCityModel.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "NavSystem/TwinLinkNavSystem.h"

void UTwinLinkUrbanPlanningViewSystem::TwinLinkShowUrbanPlanning() {
    const auto CityModel = FTwinLinkModule::Get().GetCityModel();
    if (CityModel == nullptr) {
        return;
    }

    if (ViewActor == nullptr) {
        FString ActorPath = "/PLATEAU-TwinLink/Blueprints/BP_UrbanPlanningView.BP_UrbanPlanningView_C";
        TSubclassOf<class AActor> ActorClass = TSoftClassPtr<AActor>(FSoftObjectPath(*ActorPath)).LoadSynchronous();
        ViewActor = GetWorld()->SpawnActor<AActor>(ActorClass);
    }
    MaterialCollection.Reset();
    MaterialCollectionPlane.Reset();
    AttributeIndexCollection.Reset();
    AttributeRates.Reset();
    AttributeUsages.Reset();
    GuideUsageColors.Reset();

    auto NavSystem = ATwinLinkNavSystem::GetInstance(GetWorld());

    double DemHeight = NavSystem != nullptr ? NavSystem->GetDemHeightMax() : 0;

    TArray<UPLATEAUCityObjectGroup*> CityObjects;
    CityModel->GetComponents<UPLATEAUCityObjectGroup>(CityObjects);
    for (int i = 0; i < CityObjects.Num(); i++) {
        FString Name = CityObjects[i]->GetName();
        if (!Name.StartsWith(TEXT("urf_"))) {
            continue;
        }
        const auto Obj = CityObjects[i]->GetAllRootCityObjects()[0];
        const auto AttributeRate = Obj.Attributes.AttributeMap.Find("urf:floorAreaRate");
        const auto Rate = AttributeRate != nullptr ? FCString::Atoi(*AttributeRate->StringValue) * 100 : 50;
        const auto AttributUsage = Obj.Attributes.AttributeMap.Find("urf:function");
        const auto Usage = AttributUsage != nullptr ? AttributUsage->StringValue : TEXT("UNKOWN");//UsageKeys[0];

        AddMaterials(Usage);

        TArray<FVector> OriginVertices;
        TArray<int> OriginTriangles;
        TArray<FVector> OriginNormal;
        TArray<FVector2D> OriginUVs;
        TArray<FProcMeshTangent> OriginTangents;
        UStaticMesh* OriginMesh = CityObjects[i]->GetStaticMesh();
        UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(OriginMesh, 0, 0, OriginVertices, OriginTriangles, OriginNormal, OriginUVs, OriginTangents);

        TMap<FString, int> OverlapCollection;
        for (int j = 0; j < OriginTriangles.Num() / 3; j++) {
            const auto V0 = OriginTriangles[j * 3 + 0];
            const auto V1 = OriginTriangles[j * 3 + 1];
            const auto V2 = OriginTriangles[j * 3 + 2];

            const auto K0 = (V0 < V1) ? FString::FromInt(V0) + TEXT(",") + FString::FromInt(V1) : FString::FromInt(V1) + TEXT(",") + FString::FromInt(V0);
            const auto K1 = (V1 < V2) ? FString::FromInt(V1) + TEXT(",") + FString::FromInt(V2) : FString::FromInt(V2) + TEXT(",") + FString::FromInt(V1);
            const auto K2 = (V2 < V0) ? FString::FromInt(V2) + TEXT(",") + FString::FromInt(V0) : FString::FromInt(V0) + TEXT(",") + FString::FromInt(V2);

            OverlapCollection.Add(K0, OverlapCollection.Contains(K0) ? *OverlapCollection.Find(K0) + 1 : 1);
            OverlapCollection.Add(K1, OverlapCollection.Contains(K1) ? *OverlapCollection.Find(K1) + 1 : 1);
            OverlapCollection.Add(K2, OverlapCollection.Contains(K2) ? *OverlapCollection.Find(K2) + 1 : 1);
        }

        const auto Edges = RemoveOverlapLinePath(OverlapCollection, OriginVertices);

        TArray<FVector> ProcedualVertices;
        TArray<int> ProcedualTriangles;
        TArray<FVector> ProcedualNormal;
        TArray<FVector2D> ProcedualUVs;
        TArray<FColor> ProcedualColors;
        TArray<FProcMeshTangent> ProcedualTangents;

        for (const auto& Edge : Edges) {
            const auto OffsetBase = FVector(0.0f, 0.0f, DemHeight + 10000.0f);
            const auto OffsetRate = FVector(0.0f, 0.0f, Rate * 20.0f);
            const auto OffsetStartPoint = Edge.StartPoint + OffsetBase + OffsetRate;
            const auto OffsetEndPoint = Edge.EndPoint + OffsetBase + OffsetRate;

            const auto StartPointZ = NavSystem != nullptr ? NavSystem->GetDemHeight(FVector2D(Edge.StartPoint)).value() : Edge.StartPoint.Z;
            const auto EndPointZ = NavSystem != nullptr ? NavSystem->GetDemHeight(FVector2D(Edge.EndPoint)).value() : Edge.EndPoint.Z;

            const auto StartPoint = FVector(Edge.StartPoint.X, Edge.StartPoint.Y, StartPointZ);
            const auto EndPoint = FVector(Edge.EndPoint.X, Edge.EndPoint.Y, StartPointZ);

            const auto IndexV0 = ProcedualVertices.Add(OffsetStartPoint);
            const auto IndexV1 = ProcedualVertices.Add(StartPoint);
            const auto IndexV2 = ProcedualVertices.Add(OffsetEndPoint);
            const auto IndexV3 = ProcedualVertices.Add(EndPoint);

            ProcedualTriangles.Add(IndexV0);
            ProcedualTriangles.Add(IndexV1);
            ProcedualTriangles.Add(IndexV2);
            ProcedualTriangles.Add(IndexV2);
            ProcedualTriangles.Add(IndexV1);
            ProcedualTriangles.Add(IndexV3);

            ProcedualUVs.Add(FVector2D(0.0f, 0.0f));
            ProcedualUVs.Add(FVector2D(0.0f, 1.0f));
            ProcedualUVs.Add(FVector2D(1.0f, 0.0f));
            ProcedualUVs.Add(FVector2D(1.0f, 1.0f));
        }

        USceneComponent* Parent = NewObject<USceneComponent>(ViewActor);

        Parent->SetupAttachment(ViewActor->GetRootComponent());
        Parent->RegisterComponent();

        UProceduralMeshComponent* ProcedualMesh = NewObject<UProceduralMeshComponent>(ViewActor);

        ProcedualMesh->CreateMeshSection(0, ProcedualVertices, ProcedualTriangles, ProcedualNormal, ProcedualUVs, ProcedualColors, ProcedualTangents, false);
        ProcedualMesh->SetMaterial(0, Cast<UMaterialInterface>(MaterialCollection.Find(Usage)->Get()));
        ProcedualMesh->SetupAttachment(Parent);
        ProcedualMesh->RegisterComponent();

        //マテリアルが設定されていなければ追加する
        if (OriginMesh->GetStaticMaterials().Num() == 0) {
            UMaterialInstanceDynamic* DefaultMaterial = NewObject<UMaterialInstanceDynamic>();
            OriginMesh->AddMaterial(DefaultMaterial);
        }

        int Height = (DemHeight + 10000.0f + (Rate * 20.0f)) / 100.0f * 0.1f;

        for (int j = 0; j < Height; j++) {
            UStaticMeshComponent* StaticMesh = NewObject<UStaticMeshComponent>(ViewActor);

            StaticMesh->SetStaticMesh(Cast<UStaticMesh>(CityObjects[i]->GetStaticMesh()));
            StaticMesh->AddLocalOffset(FVector(0.0f, 0.0f, DemHeight + (100.0f * j)));
            StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(MaterialCollectionPlane.Find(Usage)->Get()));
            StaticMesh->SetupAttachment(Parent);
            StaticMesh->RegisterComponent();
        }

        AttributeIndexCollection.Add(Parent, AttributeIndexCollection.Num());
        AttributeRates.Add(FString::FromInt(Rate));
        AttributeUsages.Add(Usage);
    }
}

void UTwinLinkUrbanPlanningViewSystem::TwinLinkHideUrbanPlanning() {
    if (ViewActor != nullptr) {
        ViewActor->Destroy();
        ViewActor = nullptr;
    }
}

void UTwinLinkUrbanPlanningViewSystem::TwinLinkSelectUrbanPlanning() {
    if (ViewActor == nullptr) {
        return;
    }
    const auto PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    FHitResult HitResult;
    PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult);
    if (!HitResult.Component.IsValid()) {
        return;
    }
    const auto Parent = HitResult.GetComponent()->GetAttachParent();
    if (AttributeIndexCollection.Contains(Parent)) {
        SelectAttributesIndex = AttributeIndexCollection[Parent];
    }
    else {
        SelectAttributesIndex = -1;
    }
}

FString UTwinLinkUrbanPlanningViewSystem::TwinLinkGetUrbanPlanningRate() {
    return SelectAttributesIndex < 0 ? TEXT("---") : AttributeRates[SelectAttributesIndex];
}

FString UTwinLinkUrbanPlanningViewSystem::TwinLinkGetUrbanPlanningUsage() {
    return SelectAttributesIndex < 0 ? TEXT("---") : AttributeUsages[SelectAttributesIndex];
}

TMap<FString, FColor> UTwinLinkUrbanPlanningViewSystem::GetGuideUsageColors() {
    return GuideUsageColors;
}

void UTwinLinkUrbanPlanningViewSystem::AddMaterials(const FString& NewUsage) {
    if (MaterialCollection.Contains(NewUsage)) {
        return;
    }
    int NewColorSub = MaterialCollection.Num() / OriginColors.Num();
    int NewColorMod = MaterialCollection.Num() % OriginColors.Num();
    FColor NewColor = OriginColors[NewColorMod];
    NewColor += FColor(0x08 * NewColorSub, 0x08 * NewColorSub, 0x08 * NewColorSub, 0xFF);
    GuideUsageColors.Add(NewUsage, NewColor);

    const auto ParentMaterialPath = TEXT("/PLATEAU-TwinLink/Materials/M_UrbanPlanning");
    auto Material = UMaterialInstanceDynamic::Create(Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, ParentMaterialPath)), this);
    Material->SetVectorParameterValue("EmissiveColor", NewColor);
    Material->SetScalarParameterValue("EmissveBoost", TwinLinkGraphicsEnv::GetEmissiveBoostFromEnv(GetWorld()));
    MaterialCollection.Add(NewUsage, Material);

    const auto ParentMaterialPathPlane = TEXT("/PLATEAU-TwinLink/Materials/M_UrbanPlanningPlane");
    auto MaterialPlane = UMaterialInstanceDynamic::Create(Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, ParentMaterialPathPlane)), this);
    MaterialPlane->SetVectorParameterValue("EmissiveColor", NewColor);
    MaterialPlane->SetScalarParameterValue("EmissveBoost", TwinLinkGraphicsEnv::GetEmissiveBoostFromEnv(GetWorld()));
    MaterialCollectionPlane.Add(NewUsage, MaterialPlane);
}

TArray<FTwinLinkEdge> UTwinLinkUrbanPlanningViewSystem::RemoveOverlapLinePath(TMap<FString, int> Maps, TArray<FVector> Points) {
    TArray<FTwinLinkEdge> TmpArray;
    for (const auto& LineMap : Maps) {
        if (LineMap.Value > 1) {
            continue;
        }
        TArray<FString> Parsed;
        FString Key = LineMap.Key;
        Key.ParseIntoArray(Parsed, TEXT(","), true);
        FTwinLinkEdge LinePath = FTwinLinkEdge(Points[FCString::Atoi(*Parsed[0])], Points[FCString::Atoi(*Parsed[1])]);
        TmpArray.Add(LinePath);
    }
    TArray<bool> IsOverlap;
    IsOverlap.Init(false, TmpArray.Num());
    for (int i = 0; i < TmpArray.Num(); i++) {
        for (int j = i + 1; j < TmpArray.Num(); j++) {
            if ((TmpArray[i].StartPoint == TmpArray[j].StartPoint && TmpArray[i].EndPoint == TmpArray[j].EndPoint) ||
                (TmpArray[i].StartPoint == TmpArray[j].EndPoint && TmpArray[i].EndPoint == TmpArray[j].StartPoint)) {
                IsOverlap[i] = true;
                IsOverlap[j] = true;
            }
        }
    }
    TArray<FTwinLinkEdge> RetArray;
    for (int i = 0; i < IsOverlap.Num(); i++) {
        if (IsOverlap[i]) {
            continue;
        }
        RetArray.Add(TmpArray[i]);
    }
    return RetArray;
}