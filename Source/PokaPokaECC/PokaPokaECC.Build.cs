// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PokaPokaECC : ModuleRules
{
	public PokaPokaECC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		PublicIncludePaths.AddRange(new string[] {
			"PokaPokaECC",
			"PokaPokaECC/Variant_Platforming",
			"PokaPokaECC/Variant_Platforming/Animation",
			"PokaPokaECC/Variant_Combat",
			"PokaPokaECC/Variant_Combat/AI",
			"PokaPokaECC/Variant_Combat/Animation",
			"PokaPokaECC/Variant_Combat/Gameplay",
			"PokaPokaECC/Variant_Combat/Interfaces",
			"PokaPokaECC/Variant_Combat/UI",
			"PokaPokaECC/Variant_SideScrolling",
			"PokaPokaECC/Variant_SideScrolling/AI",
			"PokaPokaECC/Variant_SideScrolling/Gameplay",
			"PokaPokaECC/Variant_SideScrolling/Interfaces",
			"PokaPokaECC/Variant_SideScrolling/UI"
		});

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
       
        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
