// Copyright (C) 2023, MLIT Japan. All rights reserved.

#include "TwinLinkPOISubSystem.h"
#include "TwinLink.h"
#include "TwinLinkCommon.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUGeometry.h"
#include <libshape/shp_reader.h>
#include "libshape/shp_writer.h"
#include <Windows.h>

void UTwinLinkPOISubSystem::ImportShapefile(const TArray<FString>& Files) {
    for (const auto& File : Files) {
        //相対パスを絶対パスに変換
        const auto AbsoluteFile = FPaths::ConvertRelativePathToFull(File);

        Layer = shp::ShpReader::read(TCHAR_TO_UTF8(*AbsoluteFile));

        const auto Fields = Layer->attribute_database.fields;
        const auto Attributes = Layer->attribute_database.attributes;
        const auto Shapes = Layer->shapes;

        //ポイント(高さ情報無し含む）以外なら中断
        if (Layer->type != shp::ShapeType::Point) {
            if (Layer->type != shp::ShapeType::PointZ) {
                return;
            }
        }

        //サイズ不一致なら中断（おそらくないはず）
        if (Attributes.size() != Shapes.size()) {
            return;
        }

        AttributeList.Reset();
        AttributeListIndex.Reset();

        for (int i = 0; i < Fields.size(); i++) {
            if (ConvertSJISToFString(Attributes[0][i]) == TEXT("")) {
                continue;
            }
            AttributeList.Add(ConvertSJISToFString(Fields[i]) + TEXT("(例:") + ConvertSJISToFString(Attributes[0][i]) + TEXT(")"));
            AttributeListIndex.Add(i);
        }

        //複数選択廃止
        break;
    }
}

void UTwinLinkPOISubSystem::RegisterAttributes(const FString& AttributeName, int AttributeIndex) {
    const auto Fields = Layer->attribute_database.fields;
    const auto Attributes = Layer->attribute_database.attributes;
    const auto Shapes = Layer->shapes;

    const auto CityModel = FTwinLinkModule::Get().GetCityModel();
    FPLATEAUGeoReference GeoReference = Cast<APLATEAUInstancedCityModel>(CityModel)->GeoReference;

    for (int i = 0; i < Attributes.size(); i++) {
        FPLATEAUGeoCoordinate Coordinate;
        Coordinate.Latitude = Shapes[i].points[0].y;
        Coordinate.Longitude = Shapes[i].points[0].x;
        Coordinate.Height = Shapes[i].points[0].z;
        FVector Point = UPLATEAUGeoReferenceBlueprintLibrary::Project(GeoReference, Coordinate);

        //都市モデル内にないポイントは削除
        FHitResult OutHit;
        GetWorld()->LineTraceSingleByChannel(OutHit, Point + FVector(0.0f, 0.0f, 100000.0f), Point, ECollisionChannel::ECC_Visibility);
        if (!OutHit.bBlockingHit) {
            continue;
        }
        Point = OutHit.ImpactPoint;

        if (RegisteredPOIs.Contains(AttributeName)) {
            RegisteredPOIs[AttributeName].Add(ConvertSJISToFString(Attributes[i][AttributeListIndex[AttributeIndex]]), Point);
        }
        else {
            TMap<FString, FVector> POIData;
            POIData.Add(ConvertSJISToFString(Attributes[i][AttributeListIndex[AttributeIndex]]), Point);
            RegisteredPOIs.Add(AttributeName, POIData);
        }
    }
}

void UTwinLinkPOISubSystem::AssignMaterials(const FString& AttributeName) {
    if (MaterialCollection.Contains(AttributeName)) {
        return;
    }
    const auto NewColor = AssignColor();

    const auto ParentMaterialPath = TEXT("/PLATEAU-TwinLink/Materials/M_POIPin");
    auto Material = UMaterialInstanceDynamic::Create(Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, ParentMaterialPath)), this);
    Material->SetVectorParameterValue("Color", NewColor);
    MaterialCollection.Add(AttributeName, Material);
}

FColor UTwinLinkPOISubSystem::AssignColor() {
    FColor NewColor = FColor(0xFF, 0xFF, 0xFF, 0xFF);

    for (int i = 0; i < RegisteredPOIs.Num(); i++) {
        int NewColorSub = i / OriginColors.Num();
        int NewColorMod = i % OriginColors.Num();
        NewColor = OriginColors[NewColorMod];
        NewColor += FColor(0x20 * NewColorSub, 0x20 * NewColorSub, 0x20 * NewColorSub, 0xFF);

        if (AssignColors.Find(NewColor) == -1) {
            break;
        }
    }
    AssignColors.Add(NewColor);

    return NewColor;
}

FString UTwinLinkPOISubSystem::ConvertSJISToFString(const std::string& SjisString) {
    // Shift-JISからUTF-16への変換
    FString Utf16String;
    int32 Utf16StringLength = MultiByteToWideChar(932, 0, SjisString.c_str(), -1, nullptr, 0);
    if (Utf16StringLength > 0) {
        TCHAR* Utf16Buffer = new TCHAR[Utf16StringLength];
        MultiByteToWideChar(932, 0, SjisString.c_str(), -1, Utf16Buffer, Utf16StringLength);
        Utf16String = FString(Utf16Buffer);
        delete[] Utf16Buffer;
    }

    return Utf16String;
}