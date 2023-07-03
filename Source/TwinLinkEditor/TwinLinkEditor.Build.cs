// Copyright (C) 2023, MLIT Japan. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class TwinLinkEditor : ModuleRules
{
    public TwinLinkEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "TwinLink",
                "TwinLinkWidgets",
                // ... add other public dependencies that you statically link with here ...
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine",
                "InputCore",
                "CoreUObject",
                "Projects",
                "LevelEditor",
                "UnrealEd",
                "SlateCore",
                "Slate",
                "Blutility",
                "UMG",
                "UMGEditor",
                "WorkspaceMenuStructure",
                // ... add private dependencies that you statically link with here ...	
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );

        //using c++17
        CppStandard = CppStandardVersion.Cpp17;
    }
}
