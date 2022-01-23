// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechModule.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Developer/Settings/Public/ISettingsContainer.h"
#include "BYGTextToSpeechSettings.h"

#define LOCTEXT_NAMESPACE "BYGTextToSpeechModule"

void FBYGTextToSpeechModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();
	
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer( "Project" );
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings( "Project", "Plugins", "BYG Text To Speech",
			LOCTEXT( "RuntimeGeneralSettingsName", "BYG Text To Speech" ),
			LOCTEXT( "RuntimeGeneralSettingsDescription", "Configure text to speech settings." ),
			GetMutableDefault<UBYGTextToSpeechSettings>()
		);
	}
}

void FBYGTextToSpeechModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
	
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		SettingsModule->UnregisterSettings( "Project", "Plugins", "BYG Text To Speech" );
	}

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGTextToSpeechModule, BYGTextToSpeech );

