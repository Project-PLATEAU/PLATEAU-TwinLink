// Copyright (C) 2023, MLIT Japan. All rights reserved.
using UnrealBuildTool;
using System;
using System.IO;

public class TwinLink : ModuleRules
{
    public TwinLink(ReadOnlyTargetRules Target) : base(Target)
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
                "Core", "CoreUObject", "Engine", "InputCore", "PLATEAURuntime", "Slate", "SlateCore","Niagara", "ProceduralMeshComponent", "HTTP", "Json"
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
                "Json",
                "JsonUtilities",
                "NavigationSystem",
                "Niagara", "Imath", 
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

        AddLib();
    }

    public void AddLib()
    {
        bEnableExceptions = true;

        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/include"));
        string libPath = Path.Combine(ModuleDirectory, "../ThirdParty/lib/libshape.lib");
        PublicAdditionalLibraries.Add(libPath);
        libPath = Path.Combine(ModuleDirectory, "../ThirdParty/lib/shp.lib");
        PublicAdditionalLibraries.Add(libPath);
    }
}
