// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;
using System.Diagnostics;
using EpicGames.Core;


public class GKModularGameplayEd : ModuleRules
{
    public GKModularGameplayEd(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory));

        // ... add other public dependencies that you statically link with here ...
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "GKModularGameplay",

            "PropertyEditor",
            "SlateCore",

            // Misc
            "GameFeatures",
            "UnrealEd",
            "CoreUObject",
            "Engine",
            "Slate",
            

            "Projects",                 // IPluginManager

            "DataTableEditor",          // Extend the table editor for AbilityData
            "LevelEditor",              // Right click option
            "Settings",                 // Add a setting page
            "EditorScriptingUtilities", // Blueprint Scripting
            "PythonScriptPlugin",       // Python Scripts
        });

        PrivateDependencyModuleNames.AddRange(new string[] {

        });

    }
}
