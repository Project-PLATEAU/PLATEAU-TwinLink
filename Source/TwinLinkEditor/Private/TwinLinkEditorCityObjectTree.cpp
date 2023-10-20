// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkEditorCityObjectTree.h"
#include "PLATEAUInstancedCityModel.h"
#include "TwinLinkCityObjectTree.h"
#include "Kismet/GameplayStatics.h"

void UTwinLinkEditorCityObjectTree::CreateCityObjectTree(UEditorActorSubsystem* Editor, UWorld* World,
    APLATEAUInstancedCityModel* CityModel, UTwinLinkEditorCityObjectTreeParam* Param) {
    checkf(Param != nullptr, TEXT("Paramがnullです"));
    if (!Param)
        return;

    checkf(Param->CityObjectTreeBp != nullptr, TEXT("UTwinLinkEditorCityObjectTreeParamのCityObjectTreeBpがnullです"));
    if (!Param->CityObjectTreeBp)
        return;

    checkf(CityModel != nullptr, TEXT("CityModelがnullです"));
    if (!CityModel)
        return;
    // 既存のTreeは破壊して作り直す
    {
        TArray<AActor*> NavSystemActors;
        UGameplayStatics::GetAllActorsOfClass(World, ATwinLinkCityObjectTree::StaticClass(), NavSystemActors);
        for (const auto& Actor : NavSystemActors)
            Actor->Destroy();
    }
    ATwinLinkCityObjectTree* Tree = Cast<ATwinLinkCityObjectTree>(Editor->SpawnActorFromObject(Param->CityObjectTreeBp, FVector::Zero(), FRotator::ZeroRotator));
    Tree->SetActorLabel(TEXT("TwinLinkCityObjectTree"));
    Tree->Init(CityModel);
}
