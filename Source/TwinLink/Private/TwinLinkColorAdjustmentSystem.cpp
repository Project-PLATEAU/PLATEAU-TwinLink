// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkColorAdjustmentSystem.h"
#include "PLATEAUInstancedCityModel.h"
#include "TwinLink.h"
#include "TwinLinkCommon.h"
#include "TwinLinkCSVExporter.h"
#include "TwinLinkCSVImporter.h"
#include "TwinLinkCSVContents.h"
#include "TwinLinkPersistentPaths.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"

void UTwinLinkColorAdjustmentSystem::ExtractMaterial() {
    const auto FacilityModel = FTwinLinkModule::Get().GetFacilityModel();
    if (FacilityModel == nullptr) {
        return;
    }

    TArray<UPLATEAUCityObjectGroup*> FacilityObjects;
    FacilityModel->GetComponents<UPLATEAUCityObjectGroup>(FacilityObjects);
    for (const auto& FacilityObject : FacilityObjects) {
        const auto OriginMaterials = FacilityObject->GetMaterials();
        for (const auto& OriginMaterial : OriginMaterials) {
            int MaterialIndex = &OriginMaterial - &OriginMaterials[0];
            if (!OriginMaterial->GetBaseMaterial()->GetName().StartsWith(TEXT("PLATEAU"))) {
                continue;
            }
            int Index = -1;
            for (const auto& Material : Materials) {
                if (OriginMaterial == Material) {
                    Index = &Material - &Materials[0];
                    break;
                }
            }
            if (Index == -1) {
                Index = Materials.Add(OriginMaterial);
                MaterialParameter.Add(FTwinLinkAdjustmentMaterialParameter(TEXT("マテリアル" + FString::FromInt(Index + 1)), Index, OriginMaterial));
                MaterialParameterOrigin.Add(FTwinLinkAdjustmentMaterialParameter(TEXT("マテリアル" + FString::FromInt(Index + 1)), Index, OriginMaterial));

                //Specular/Metallicを1.0に強制
                Cast<UMaterialInstanceDynamic>(Materials[Index])->SetScalarParameterValue(FName(TEXT("Specular/Metallic")), 1.0f);
            }
            MaterialExportData.Add(FTwinLinkAdjustmentMaterialExport(FacilityModel.GetName(), FacilityObject->GetName(), FString::FromInt(MaterialIndex), Index));
        }
    }

    //エミッシブ強度調整
    TArray<TObjectPtr<AActor>> Actors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
    for (const auto& Actor : Actors) {
        const auto SkyLight = Actor->GetComponentByClass(USkyLightComponent::StaticClass());
        if (SkyLight != nullptr) {
            const auto Component = Cast<USceneComponent>(Actor->GetComponentByClass(UDirectionalLightComponent::StaticClass()));
            //ライトのYが+の場合は夜と判定
            const auto Night = Component->GetRelativeRotation().Pitch > 0;
            EmissibeStrength = Night ? 100 : 1000000;
            break;
        }
    }

    Filepath = TwinLinkPersistentPaths::CreateViewPointFilePath(TEXT("ColorAdjustmentInfo.csv"));

    ImportColorAdjustmentParam();
}

void UTwinLinkColorAdjustmentSystem::ExportColorAdjustmentParam() {
    UE_TWINLINK_LOG(LogTemp, Log, TEXT("TwinLink ExportAssetPlacementInfo : %s"), *Filepath);

    TwinLinkCSVExporter CSVExporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    CSVExporter.SetHeaderContents(
        CSVContents.CreateHeaderContents(
            TEXT("actor_name,component_name,material_index,default_diffuse_r,default_diffuse_g,default_diffuse_b,default_emissive_r,default_emissive_g,default_emissive_b,default_specular_r,default_specular_g,default_specular_b,default_shininess,default_transparency,diffuse_r,diffuse_g,diffuse_b,emissive_r,emissive_g,emissive_b,specular_r,specular_g,specular_b,shininess,transparency")));

    for (const auto& ExportData : MaterialExportData) {
        const auto StringBuf = CSVContents.CreateBodyContents(
            FString::Printf(TEXT("%s,%s,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"),
                *ExportData.ActorName, *ExportData.ComponentName, *ExportData.MaterialIndex,
                MaterialParameterOrigin[ExportData.MaterialParamIndex].BaseColor.R, MaterialParameterOrigin[ExportData.MaterialParamIndex].BaseColor.G, MaterialParameterOrigin[ExportData.MaterialParamIndex].BaseColor.B,
                MaterialParameterOrigin[ExportData.MaterialParamIndex].EmissiveColor.R, MaterialParameterOrigin[ExportData.MaterialParamIndex].EmissiveColor.G, MaterialParameterOrigin[ExportData.MaterialParamIndex].EmissiveColor.B,
                MaterialParameterOrigin[ExportData.MaterialParamIndex].SpecularColor.R, MaterialParameterOrigin[ExportData.MaterialParamIndex].SpecularColor.G, MaterialParameterOrigin[ExportData.MaterialParamIndex].SpecularColor.B,
                MaterialParameterOrigin[ExportData.MaterialParamIndex].Shininess, MaterialParameterOrigin[ExportData.MaterialParamIndex].Transparency,
                MaterialParameter[ExportData.MaterialParamIndex].BaseColor.R, MaterialParameter[ExportData.MaterialParamIndex].BaseColor.G, MaterialParameter[ExportData.MaterialParamIndex].BaseColor.B,
                MaterialParameter[ExportData.MaterialParamIndex].EmissiveColor.R, MaterialParameter[ExportData.MaterialParamIndex].EmissiveColor.G, MaterialParameter[ExportData.MaterialParamIndex].EmissiveColor.B,
                MaterialParameter[ExportData.MaterialParamIndex].SpecularColor.R, MaterialParameter[ExportData.MaterialParamIndex].SpecularColor.G, MaterialParameter[ExportData.MaterialParamIndex].SpecularColor.B,
                MaterialParameter[ExportData.MaterialParamIndex].Shininess, MaterialParameter[ExportData.MaterialParamIndex].Transparency));

        CSVExporter.AddBodyContents(StringBuf);
    }

    const auto bIsSuc = CSVExporter.ExportCSV(*Filepath);
    check(bIsSuc);
}

void UTwinLinkColorAdjustmentSystem::ImportColorAdjustmentParam() {
    TwinLinkCSVImporter CSVImporter;
    TwinLinkCSVContents::Standard CSVContents(VersionInfo);
    const auto bIsSuc = CSVImporter.ImportCSV(*Filepath, &CSVContents);

    if (bIsSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Failed Import CSV : %s"), *Filepath);
        return;
    }

    // データを解析して取得する
    const auto& BodyContents = CSVContents.GetBodyContents();

    for (const auto& BodyContentsElement : BodyContents) {
        // データを解析する
        if (!Parse(BodyContentsElement)) {
            // エラーデータを出力する デバッグ用
            for (const auto& ErrorStrDataElement : BodyContentsElement) {
                UE_TWINLINK_LOG(LogTemp, Log, TEXT("Contains invalid data : %s"), *ErrorStrDataElement);
            }
        }
    }
}

bool UTwinLinkColorAdjustmentSystem::Parse(const TArray<FString>& StrInfo) {
    // プリミティブな型単位での要素数 FVectorならfloatが3つなので　25
    constexpr auto NumPrimElementOfViewPointInfoStruct = 25;
    if (StrInfo.Num() != NumPrimElementOfViewPointInfoStruct) {
        // 引数文字列が正しくない
        return false;
    }

    int RefIdx = 0;
    FString Actor, Component, Index;
    FLinearColor OriginBaseColor, OriginEmissiveColor, OriginSpecularColor;
    float OriginShininess, OriginTransparency;
    FLinearColor BaseColor, EmissiveColor, SpecularColor;
    float Shininess, Transparency;

    Actor = StrInfo[RefIdx++];
    Component = StrInfo[RefIdx++];
    Index = StrInfo[RefIdx++];

    OriginBaseColor = FLinearColor(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2]));   RefIdx += 3;
    OriginEmissiveColor = FLinearColor(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2]));   RefIdx += 3;
    OriginSpecularColor = FLinearColor(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2]));   RefIdx += 3;
    OriginShininess = FCString::Atof(*StrInfo[RefIdx++]);
    OriginTransparency = FCString::Atof(*StrInfo[RefIdx++]);

    BaseColor = FLinearColor(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2]));   RefIdx += 3;
    EmissiveColor = FLinearColor(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2]));   RefIdx += 3;
    SpecularColor = FLinearColor(FCString::Atof(*StrInfo[RefIdx]), FCString::Atof(*StrInfo[RefIdx + 1]), FCString::Atof(*StrInfo[RefIdx + 2]));   RefIdx += 3;
    Shininess = FCString::Atof(*StrInfo[RefIdx++]);
    Transparency = FCString::Atof(*StrInfo[RefIdx++]);

    for (const auto& ExportData : MaterialExportData) {
        if (ExportData.ActorName.Equals(Actor) && ExportData.ComponentName.Equals(Component) && ExportData.MaterialIndex.Equals(Index)) {
            MaterialParameterOrigin[ExportData.MaterialParamIndex].BaseColor = OriginBaseColor;
            MaterialParameterOrigin[ExportData.MaterialParamIndex].EmissiveColor = OriginEmissiveColor;
            MaterialParameterOrigin[ExportData.MaterialParamIndex].SpecularColor = OriginSpecularColor;
            MaterialParameterOrigin[ExportData.MaterialParamIndex].Shininess = OriginShininess;
            MaterialParameterOrigin[ExportData.MaterialParamIndex].Transparency = OriginTransparency;

            MaterialParameter[ExportData.MaterialParamIndex].BaseColor = BaseColor;
            MaterialParameter[ExportData.MaterialParamIndex].EmissiveColor = EmissiveColor;
            MaterialParameter[ExportData.MaterialParamIndex].SpecularColor = SpecularColor;
            MaterialParameter[ExportData.MaterialParamIndex].Shininess = Shininess;
            MaterialParameter[ExportData.MaterialParamIndex].Transparency = Transparency;

            Cast<UMaterialInstanceDynamic>(Materials[ExportData.MaterialParamIndex])->SetVectorParameterValue(FName(TEXT("BaseColor")), BaseColor);
            Cast<UMaterialInstanceDynamic>(Materials[ExportData.MaterialParamIndex])->SetVectorParameterValue(FName(TEXT("SpecularColor")), SpecularColor);
            Cast<UMaterialInstanceDynamic>(Materials[ExportData.MaterialParamIndex])->SetVectorParameterValue(FName(TEXT("EmissiveColor")), EmissiveColor * EmissibeStrength);
            Cast<UMaterialInstanceDynamic>(Materials[ExportData.MaterialParamIndex])->SetScalarParameterValue(FName(TEXT("Shininess")), Shininess);
            Cast<UMaterialInstanceDynamic>(Materials[ExportData.MaterialParamIndex])->SetScalarParameterValue(FName(TEXT("Transparency")), Transparency);
        }
    }

    return true;
}