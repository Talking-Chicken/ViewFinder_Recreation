// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Viewfinder : ModuleRules
{
	public Viewfinder(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Viewfinder",
			"Viewfinder/Variant_Horror",
			"Viewfinder/Variant_Horror/UI",
			"Viewfinder/Variant_Shooter",
			"Viewfinder/Variant_Shooter/AI",
			"Viewfinder/Variant_Shooter/UI",
			"Viewfinder/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
