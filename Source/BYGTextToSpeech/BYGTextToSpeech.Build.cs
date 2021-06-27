using UnrealBuildTool;
using System.IO;

public class BYGTextToSpeech : ModuleRules
{
	public BYGTextToSpeech(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Media",
				//"Slate",
				//"UnrealEd",
                //"UMG",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
                //"SlateCore",
                //"EditorStyle",
                //"Projects",
				//"PropertyEditor",
				//"EditorSubsystem",
				//"EditorWidgets",
				"DeveloperSettings",
				//"RenderCore",
            }
        );

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);


		// Load FMRTTSLib
		if (Target.Platform != UnrealTargetPlatform.Win64 && Target.Platform != UnrealTargetPlatform.Win32)
		{
			PublicDefinitions.Add("WITH_FMRTTSLIB=0");
		}
		else
		{
			var FMRTTSLibFolder = Path.Combine(ModuleDirectory, @"..\..\ThirdParty", "FMRTTSLib");
			var LibraryPath = Path.Combine(FMRTTSLibFolder, Target.Platform == UnrealTargetPlatform.Win64 ? "x64" : "x86");
			
			PublicSystemLibraryPaths.Add(LibraryPath);
			
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "FMRTTSLib.lib"));
			PrivateIncludePaths.Add(Path.Combine(FMRTTSLibFolder, "include"));

			PublicDefinitions.Add("WITH_FMRTTSLIB=1");
		}
	}
}
