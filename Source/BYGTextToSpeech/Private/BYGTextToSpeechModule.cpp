// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechModule.h"

#define LOCTEXT_NAMESPACE "BYGTextToSpeechModule"

void FBYGTextToSpeechModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();
}

void FBYGTextToSpeechModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGTextToSpeechModule, BYGTextToSpeech );


