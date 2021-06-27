// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechEditorModule.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
//#include "Developer/Settings/Public/ISettingsContainer.h"
#include "BYGTextToSpeechSettings.h"

#define LOCTEXT_NAMESPACE "BYGTextToSpeechEditorModule"

void FBYGTextToSpeechEditorModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();

	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings( "Project", "Plugins", "BYGTextToSpeech",
			LOCTEXT( "RuntimeGeneralSettingsName", "BYG Text to Speech" ),
			LOCTEXT( "RuntimeGeneralSettingsDescription", "Speaks text using the Windows Speech API." ),
			GetMutableDefault<UBYGTextToSpeechSettings>()
		);
	}
}

void FBYGTextToSpeechEditorModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();

	if ( UObjectInitialized() )
	{
		if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
		{
			SettingsModule->UnregisterSettings( "Project", "Plugins", "BYGTextToSpeech" );
		}
	}

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGTextToSpeechEditorModule, BYGTextToSpeechEditor );

