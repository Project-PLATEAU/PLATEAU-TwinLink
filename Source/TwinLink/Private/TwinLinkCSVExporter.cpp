// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkCSVExporter.h"
#include "Misc/FileHelper.h"

TwinLinkCSVExporter::TwinLinkCSVExporter()
{
}

TwinLinkCSVExporter::~TwinLinkCSVExporter()
{
}

void TwinLinkCSVExporter::SetHeaderContents(const FString Header) {
    HeaderContents = Header;
}

void TwinLinkCSVExporter::SetHeaderContents(const TArray<FString> Header) {
    HeaderContents = FString();
    for (auto& content : Header) {
        HeaderContents.Append(content);
        HeaderContents.Append(TEXT(","));
    }
    HeaderContents.RemoveAt(HeaderContents.Len() - 1, FString(TEXT(",")).Len());
}

void TwinLinkCSVExporter::AddBodyContents(const FString Contents) {
    BodyContents.Append(Contents);
    BodyContents.Append(TEXT("\n"));
}

bool TwinLinkCSVExporter::ExportCSV(const TCHAR* Filepath) {
    BodyContents.RemoveFromEnd(TEXT("\n"));
    const FString CSVContents = HeaderContents + FString("\n") + BodyContents;
    return FFileHelper::SaveStringToFile(CSVContents, Filepath);
}

void TwinLinkCSVExporter::ResetBodyContents() {
    BodyContents.Empty();
}

bool TwinLinkCSVExporter::IsHasBodyContents() {
    return BodyContents.IsEmpty() == false;
}
