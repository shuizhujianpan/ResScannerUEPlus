// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ResScannerEditor : ModuleRules
{
	public ResScannerEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"ResScanner",
				"Json",
				"JsonUtilities",
				"Projects",
				"DesktopPlatform",
				"GitSourceControlEx",
				"PropertyEditor"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"EditorStyle",
				"InputCore",
				"HTTP"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		PublicDefinitions.AddRange(new string[]
		{
			"TOOL_NAME=\"ResScannerUE\"",
			"CURRENT_VERSION_ID=24",
			"REMOTE_VERSION_FILE=\"https://imzlp.com/opensource/version.json\""
		});
	}
}
