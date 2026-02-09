// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CrimsonMoon : ModuleRules
{
	public CrimsonMoon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"GameplayTasks",
			"GameplayAbilities",
			"AnimGraphRuntime",
			"AIModule",
			"UMG",
			"Slate",
			"SlateCore",
			"MotionWarping",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"OnlineSubsystem",
			"OnlineSubsystemSteam",
			"SteamSockets",
			"NavigationSystem",
			"GameplayCameras" 
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"NetCore", "AnimGraphRuntime"// ENetCloseResult / LexToString 정의를 위해 추가
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
