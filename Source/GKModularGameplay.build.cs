// Copyright 2024 Mischievous Game, Inc. All Rights Reserved.

using System;
using System.IO;

using UnrealBuildTool;

public class GKModularGameplay : ModuleRules
{
    public GKModularGameplay(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrecompileForTargets = PrecompileTargetsType.Any;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory));

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",       // AssetManager

                // Main Plugins
                "ModularGameplay",
                "ModularGameplayActors",
                "GameFeatures",
                "GameplayAbilities",    // HUD needs this to fetch debug actors
                "GameplayTags",         // Tags
                "UMG",                  // Widget
                "CommonLoadingScreen",  // Loading Screen
                "CommonGame",           // UCommonActivatableWidget
                "UIExtension",          // Add Widget Action
                "AIModule",             // Generic Team Interface
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "EnhancedInput",    // Input Action
                "CommonUI",         // Add Widget Action
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
            }
        );

        // Version Info
        // ------------
        // Automatically set by the CI
        string GKMODULARGAMEPLAY_TAG = "v0.0.0";
        string GKMODULARGAMEPLAY_HASH = "outoftree";
        string GKMODULARGAMEPLAY_DATE = "1900-01-01 00:00:00 -0000";

        PublicDefinitions.Add("GKMODULARGAMEPLAY_TAG=" + GKMODULARGAMEPLAY_TAG);
        PublicDefinitions.Add("GKMODULARGAMEPLAY_COMMIT=" + GKMODULARGAMEPLAY_HASH);
        PublicDefinitions.Add("GKMODULARGAMEPLAY_DATE=" + GKMODULARGAMEPLAY_DATE);
    }
}
