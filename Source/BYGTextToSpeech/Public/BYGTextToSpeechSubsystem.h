// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "Widgets/SWidget.h"
#include "BYGTextToSpeechSubsystem.generated.h"

// Copied from landscape subsystem
struct FBYGTextToSpeechSubsystemTickFunction : public FTickFunction
{
	FBYGTextToSpeechSubsystemTickFunction() : FBYGTextToSpeechSubsystemTickFunction( nullptr ) {}
	FBYGTextToSpeechSubsystemTickFunction( class UBYGTextToSpeechSubsystem* InSubsystem ) : Subsystem( InSubsystem ) {}
	virtual ~FBYGTextToSpeechSubsystemTickFunction() {}

	// Begin FTickFunction overrides
	virtual void ExecuteTick( float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent ) override;
	virtual FString DiagnosticMessage() override;
	virtual FName DiagnosticContext( bool bDetailed ) override;
	// End FTickFunction overrides

	class UBYGTextToSpeechSubsystem* Subsystem;
};

UCLASS()
class BYGTEXTTOSPEECH_API UBYGTextToSpeechSubsystem : public UWorldSubsystem, public FTickFunction //public FTickableGameObject //, public FGCObject
{
	GENERATED_BODY()

public:
	UBYGTextToSpeechSubsystem();
	virtual ~UBYGTextToSpeechSubsystem();

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	// Call this to re-trigger audio being read
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
	void Tick( float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent );

	friend struct FBYGTextToSpeechSubsystemTickFunction;
	FBYGTextToSpeechSubsystemTickFunction TickFunction;

	/** The last frame number we were ticked.  We don't want to tick multiple times per frame */
	uint32 LastFrameNumberWeTicked;

	bool bIsEnabled = true;

	// We're assuming that the installed voice name is unique
	FString VoiceName;

	bool bAutoReadOnHover = true;
	float VolumeMultiplier = 1.0f;
	float Speed = 0.5f;

	TArray<FString> LastTextWeSpoke;
	TWeakPtr<SWidget> LastParentWidgetWeSpoke;

	class UAudioComponent* AudioComponent = nullptr;
};

