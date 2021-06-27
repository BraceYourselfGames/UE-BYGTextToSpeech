// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "UnrealEd.h"
#include "Engine.h"

class FBYGTextToSpeechEditorModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
