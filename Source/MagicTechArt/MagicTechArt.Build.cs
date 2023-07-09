// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MagicTechArt : ModuleRules
{
	public MagicTechArt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Json",
			"JsonUtilities",
			"Niagara" });
	}
}
