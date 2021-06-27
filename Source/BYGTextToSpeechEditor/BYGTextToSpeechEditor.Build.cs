using UnrealBuildTool;

public class BYGTextToSpeechEditor : ModuleRules
{
	public BYGTextToSpeechEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
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
				"BYGTextToSpeech"
				//"RenderCore",
			}
        );
	}
}
