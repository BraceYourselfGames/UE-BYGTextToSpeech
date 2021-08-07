// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

BYGTEXTTOSPEECH_API DECLARE_LOG_CATEGORY_EXTERN( LogBYGTextToSpeech, Log, All );

class BYGTEXTTOSPEECH_API FBYGTextToSpeechModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
};
