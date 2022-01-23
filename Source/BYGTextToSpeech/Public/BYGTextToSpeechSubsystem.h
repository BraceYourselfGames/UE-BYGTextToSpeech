// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "Widgets/SWidget.h"
#include "BYGTextToSpeech/Private/BYGTextToSpeechRunnable.h"
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
	virtual bool ShouldCreateSubsystem( UObject* Outer ) const override;
	virtual void Initialize( FSubsystemCollectionBase& Collection ) override;
	virtual void Deinitialize() override;
	// End USubsystem

	// Kills any text being read
	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	void StopAudio();

	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	bool GetIsEnabled() const { return bIsEnabled; }
	// Disabling the plugin will stop any audio being read and kill any queued audio processing
	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	void SetIsEnabled( bool bInIsEnabled );

	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	bool GetIsAutoReadOnHoverEnabled() const { return bAutoReadOnHover; }
	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	void SetAutoReadOnHoverEnabled( bool bInAutoRead ) { bAutoReadOnHover = bInAutoRead; }

	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	float GetVolumeMultiplier() const { return VolumeMultiplier; }
	// Expected range 0~1
	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	void SetVolumeMultiplier( float InVolumeMultiplier ) { VolumeMultiplier = InVolumeMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	bool SetVoiceId( const FString& InVoiceId );

	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	float GetSpeed() const { return Speed; }
	// Expected range 0~1
	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	void SetSpeed( float InSpeed ) { Speed = FMath::Clamp<float>( InSpeed, 0.0f, 1.0f ); }

	// Adds text to the queue of things to be spoken
	UFUNCTION(BlueprintCallable, Category = "BYG Text To Speech | Subsystem")
	bool SpeakText( const TArray<FString>& Text, bool bStopExisting = true );

protected:
	void Tick( float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent );

	void CreateRunnable();
	void CleanUpRunnable();
	

	TArray<TUniquePtr<FBYGSoundWaveData>> SoundWaveDataQueue;

	UFUNCTION()
	void OnAudioFinished( UAudioComponent* AudioComp );

	float DurationRemaining = INDEX_NONE;
	bool bIsPlayingAudio = false;

	friend struct FBYGTextToSpeechSubsystemTickFunction;
	FBYGTextToSpeechSubsystemTickFunction TickFunction;

	/** The last frame number we were ticked.  We don't want to tick multiple times per frame */
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

	bool bIsEnabled = true;

	TArray<const TCHAR*> SplitDelims;

	// We're assuming that the installed voice name is unique
	FString VoiceName = "";

	bool bShowDebugLogs;
	bool bAutoReadOnHover;
	float VolumeMultiplier = 1.0f;
	float Speed = 0.5f;

	FText LastTextWeSpoke;
	TWeakPtr<SWidget> LastParentWidgetWeSpoke;

	UPROPERTY()
	class UAudioComponent* AudioComponent;

	TUniquePtr<FBYGTextToSpeechRunnable> TextToSpeechRunnable = nullptr;
};


