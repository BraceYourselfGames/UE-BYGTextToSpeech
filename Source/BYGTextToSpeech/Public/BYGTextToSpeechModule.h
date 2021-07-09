// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "UnrealEd.h"
#include "Engine.h"
#include "Tickable.h"

class FBYGTextToSpeechModule : public FDefaultGameModuleImpl, public FTickableGameObject
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void Tick( float DeltaTime ) override;

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( UBUITween, STATGROUP_Tickables );
	}
	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}
	virtual bool IsTickableInEditor() const
	{
		return false;
	}

protected:
	/** The last frame number we were ticked.  We don't want to tick multiple times per frame */
	uint32 LastFrameNumberWeTicked;

	TArray<FText> LastTextWeSpoke;
	TWeakPtr<SWidget> LastParentWidgetWeSpoke;

};
