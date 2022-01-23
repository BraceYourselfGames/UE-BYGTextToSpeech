// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class BYGTEXTTOSPEECH_API FBYGTextToSpeechModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
