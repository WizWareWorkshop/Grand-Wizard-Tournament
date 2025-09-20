using UnrealBuildTool;
using System;
using System.IO;

public class GrimoirePlugin : ModuleRules
{
    public GrimoirePlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "GameplayTags",
            "Heart",
            "HeartCore",
            "Flakes",
            "GameplayAbilities",
            "EnhancedInput",
            "Niagara",
            "StructUtils"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "PropertyEditor",
		    "GraphEditor",
            "HeartNet",
            "NetCore"

        });

        // Dynamically load Heart Graph if available
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
		    {
		        "HeartEditor",
		        "UnrealEd"
		    });
        }
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}