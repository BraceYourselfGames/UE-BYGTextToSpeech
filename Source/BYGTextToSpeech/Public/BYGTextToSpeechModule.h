// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "UnrealEd.h"
#include "IAssetTools.h"
#include "Engine.h"

class FBYGTextToSpeechModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
