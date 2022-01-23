using UnrealBuildTool;
using System.IO;

public class BYGTextToSpeech : ModuleRules
{
	public BYGTextToSpeech(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoSharedPCHs;
		PrivatePCHHeaderFile = "Public/BYGTextToSpeechModule.h";

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Media",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"InputCore",
			}
		);

		// Load atls.lib
		if (Target.Platform != UnrealTargetPlatform.Win64
#if !UE_5_0_OR_LATER
		    && Target.Platform != UnrealTargetPlatform.Win32
#endif
		)
		{
			PublicDefinitions.Add("WITH_ATLSLIB=0");
		}
		else
		{
			// If the Target.WindowsPlatform.ToolChainDir line doesn't work then manually specify the path
			var atlmfcFolder = Path.Combine(Target.WindowsPlatform.ToolChainDir, "atlmfc");
			var libFolder = Path.Combine(atlmfcFolder, "lib");
			var libraryPath = Path.Combine(libFolder, Target.Platform == UnrealTargetPlatform.Win64 ? "x64" : "x86");

			PublicSystemLibraryPaths.Add(libraryPath);
			PublicAdditionalLibraries.Add(Path.Combine(libraryPath, @"atls.lib"));
			PrivateIncludePaths.Add(Path.Combine(atlmfcFolder, "include"));

			PublicDefinitions.Add("WITH_ATLSLIB=1");
		}
	}
}