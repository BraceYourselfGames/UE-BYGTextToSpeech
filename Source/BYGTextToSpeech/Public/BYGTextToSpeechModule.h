// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Tickable.h"
#include "UObject/GCObject.h"
#include "Widgets/SWidget.h"

class BYGTEXTTOSPEECH_API FBYGTextToSpeechModule : public FDefaultGameModuleImpl, public FTickableGameObject, public FGCObject
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

	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;

	// If the user wants to re-trigger audio being read
	void ReadTextUnderCursor();
	void StopAudio();

	// Disabling will stop any audio
	void SetIsEnabled( bool bInIsEnabled );
	void SetAutoReadOnHoverEnabled( bool bInAutoRead ) { bAutoReadOnHover = bInAutoRead; }
	void SetVolumeMultiplier( float InVolumeMultiplier ) { VolumeMultiplier = InVolumeMultiplier; }
	bool SetVoiceId( const FString& InVoiceId );
	void SetSpeed( float InSpeed )
	{
		Speed = FMath::Clamp<float>( InSpeed, 0.0f, 1.0f );
	}

protected:
	UWorld* GetWorld();

	bool bIsEnabled = true;

	/** The last frame number we were ticked.  We don't want to tick multiple times per frame */
	uint32 LastFrameNumberWeTicked;

	// We're assuming that the installed voice name is unique
	FString VoiceName;

	bool bAutoReadOnHover = true;
	float VolumeMultiplier = 1.0f;
	float Speed = 0.5f;

	TArray<FString> LastTextWeSpoke;
	TWeakPtr<SWidget> LastParentWidgetWeSpoke;

	class UAudioComponent* AudioComponent = nullptr;
};
