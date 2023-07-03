// Copyright (C) 2023, MLIT Japan. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class TwinLinkWidgets : ModuleRules
{
    public TwinLinkWidgets(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicIncludePaths.AddRange(
            new string[]
            {
            }
        );
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));


        PrivateIncludePaths.AddRange(
            new string[]
            {
                // ... add other private include paths required here ...
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "CoreUObject", "Engine", "InputCore",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Projects",
                "UMG",
                "TwinLink"
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
