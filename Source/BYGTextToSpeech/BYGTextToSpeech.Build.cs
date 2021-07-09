using UnrealBuildTool;
using System.IO;

public class BYGTextToSpeech : ModuleRules
{
	public BYGTextToSpeech(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Media",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Slate",
                "SlateCore",
                "UnrealEd",
            }
        );

		// Load atls.lib
		if (Target.Platform != UnrealTargetPlatform.Win64 && Target.Platform != UnrealTargetPlatform.Win32)
		{
			PublicDefinitions.Add("WITH_ATLSLIB=0");
		}
		else
		{
			var LibFolder = Path.Combine(ModuleDirectory, @"..\..\ThirdParty");
			var LibraryPath = Path.Combine(LibFolder, Target.Platform == UnrealTargetPlatform.Win64 ? "x64" : "x86");

			PublicSystemLibraryPaths.Add( LibraryPath );

			// This sucks, I wish there was a way to do this w/o an explicit path like this. Otherwise I get atlbase.h not found
			PrivateIncludePaths.Add( @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\atlmfc\include" );

			PublicDefinitions.Add("WITH_ATLSLIB=1");
		}
	}
}
