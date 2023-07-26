// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkSystemVersionInfo.h"

TwinLinkSystemVersionInfo::TwinLinkSystemVersionInfo(int Major, int Minor, int Patch)
    : Major(Major)
    , Minor(Minor)
    , Patch(Patch){
}

TwinLinkSystemVersionInfo::TwinLinkSystemVersionInfo(FString Major, FString Minor, FString Patch) 
    : Major(FCString::Atoi(*Major))
    , Minor(FCString::Atoi(*Minor))
    , Patch(FCString::Atoi(*Patch)) {
}

TwinLinkSystemVersionInfo::~TwinLinkSystemVersionInfo()
{
}

bool TwinLinkSystemVersionInfo::CheckCompatibility(const TwinLinkSystemVersionInfo& Version) const {
    return Major == Version.Major;
}

bool TwinLinkSystemVersionInfo::CheckSameArchitecture(const TwinLinkSystemVersionInfo& Version) const {
    return Major == Version.Major && Minor == Version.Minor;
}

bool TwinLinkSystemVersionInfo::CheckCompleteMatch(const TwinLinkSystemVersionInfo& Version) const {
    return Major == Version.Major && Minor == Version.Minor && Patch == Version.Patch;
}

FString TwinLinkSystemVersionInfo::ToString() const { 
    return FString::Printf(TEXT("%i.%i.%i"), Major, Minor, Patch);
}
