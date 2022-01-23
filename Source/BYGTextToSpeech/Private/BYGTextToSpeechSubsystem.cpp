// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "BYGTextToSpeechStatics.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "BYGTextToSpeechRunnable.h"
#include "BYGTextToSpeechLog.h"

//DECLARE_CYCLE_STAT(TEXT("BYGTextToSpeechSubsystem Tick"), STAT_BYGTextToSpeechSubsystemTick, STATGROUP_BYGTextToSpeech);

UBYGTextToSpeechSubsystem::UBYGTextToSpeechSubsystem()
	: TickFunction( this )
{
}

UBYGTextToSpeechSubsystem::~UBYGTextToSpeechSubsystem()
{
	CleanUpRunnable();
}

// Thanks to @GlassBeaver and @TankNoMore for this
bool UBYGTextToSpeechSubsystem::ShouldCreateSubsystem( UObject* Outer ) const
{
	// We only want to instantiate and tick if we're actually playing the game, not if the editor is just running

	// Not sure what this is but it's named "/Temp/Untitled" and only has the RF_Transactional object flag set
	// indicates that we're in the editor but not in PIE
	if ( Outer->GetFlags() == RF_Transactional )
		return false;

	const UWorld* World = Outer->GetWorld();
	return World && ( World->WorldType == EWorldType::Type::PIE || World->WorldType == EWorldType::Type::Game );
}

void UBYGTextToSpeechSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	// Set up from project settings
	const UBYGTextToSpeechSettings& Settings = *GetDefault<UBYGTextToSpeechSettings>();
	bIsEnabled = Settings.bEnabled;
	bAutoReadOnHover = Settings.bAutoReadOnHover;
	bShowDebugLogs = Settings.bShowDebugLogs;
	VolumeMultiplier = Settings.DefaultVolumeMultiplier;
	Speed = FMath::GetMappedRangeValueClamped( FVector2D( 0, 10 ), FVector2D( 0, 1 ), Settings.DefaultRate );
	for ( const FString& Str : Settings.TextSplitDelimiters )
	{
		SplitDelims.Add( *Str );
	}
	
	// Register Tick 
	TickFunction.bCanEverTick = true;
	TickFunction.bTickEvenWhenPaused = true;
	TickFunction.bStartWithTickEnabled = true;
	TickFunction.TickGroup = TG_DuringPhysics;
	TickFunction.bAllowTickOnDedicatedServer = false;
	TickFunction.RegisterTickFunction( GetWorld()->PersistentLevel );

	CreateRunnable();
}

void UBYGTextToSpeechSubsystem::Deinitialize()
{
	CleanUpRunnable();
	TickFunction.UnRegisterTickFunction();
}

void UBYGTextToSpeechSubsystem::CreateRunnable()
{
	if ( !TextToSpeechRunnable.IsValid() )
	{
		TextToSpeechRunnable = MakeUnique<FBYGTextToSpeechRunnable>();
	}
}

void UBYGTextToSpeechSubsystem::CleanUpRunnable()
{
	if ( TextToSpeechRunnable.IsValid() )
	{
		TextToSpeechRunnable->Exit();
		TextToSpeechRunnable.Reset();
	}
}


void UBYGTextToSpeechSubsystem::Tick( float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent )
{
	//SCOPE_CYCLE_COUNTER(STAT_BYGTextToSpeechSubsystemTick);
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick );
	//TRACE_CPUPROFILER_EVENT_SCOPE(UBYGTextToSpeechWorldSubsystem::Tick);
	//CSV_SCOPED_TIMING_STAT_EXCLUSIVE(BYGTextToSpeech);
	//LLM_SCOPE(ELLMTag::BYGTextToSpeech);

	// Don't want to tick in-editor
	if ( LastFrameNumberWeTicked == GFrameCounter )
		return;

	LastFrameNumberWeTicked = GFrameCounter;

	if ( !bIsEnabled )
		return;

	if ( VoiceName.IsEmpty() )
	{
		UE_LOG( LogBYGTextToSpeech, Warning, TEXT( "Text to speech subsystem is enabled, but no voice ID set" ) );
		return;
	}

	if ( DurationRemaining > 0 )
	{
		DurationRemaining -= DeltaTime;
		if ( DurationRemaining <= 0 )
		{
			if ( bShowDebugLogs )
			{
				GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Yellow, TEXT( "TIMED OUT" ) );
			}
			bIsPlayingAudio = false;
		}
	}

	GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Yellow, FString::Printf( TEXT( "Duration remaining: %f" ), DurationRemaining ) );

	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_IfIsEnabled );
	if ( bAutoReadOnHover )
	{
		// Find the widget that we are over, and any text widgets that beneath it
		const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
		const FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse( CursorPos, FSlateApplication::Get().GetInteractiveTopLevelWindows() );

		if ( bShowDebugLogs )
		{
			GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Green, FString::Printf( TEXT( "Runnable Data: %d" ), TextToSpeechRunnable->GetSoundWaveDataSize() ) );
			GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Magenta, FString::Printf( TEXT( "Runnable Text: %d" ), TextToSpeechRunnable->GetTextQueueSize() ) );
		}

		static const FString RootName = "SGameLayerManager";

		// Anything inside here, and we're hovering over one of our widgets
		FText TextToSpeak;
		if ( WidgetPath.IsValid() )
		{
			QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_AllText );
			for ( int32 i = 0; i < WidgetPath.Widgets.Num(); ++i )
			{
				const FArrangedWidget& ArrangedWidget = WidgetPath.Widgets[ i ];
				const TSharedRef<SWidget> Widget = ArrangedWidget.Widget;

				// We are inside the window that we care about
				if ( Widget->GetTypeAsString() == RootName )
				{
					const TSharedRef<SWidget> LeafWidget = WidgetPath.GetLastWidget();
					//const FText Text = LeafWidget->GetAccessibleText( EAccessibleType::Summary );
					//TextToSpeak = LeafWidget->GetAccessibleSummary();
					TextToSpeak = LeafWidget->GetAccessibleText( EAccessibleType::Main );

					//if ( !Text.IsEmptyOrWhitespace() )
					//{
					//AllText.Add( Text.ToString() );
					//}
					//GetChildrenAccessibleText( LeafWidget, AllText );
					break;
				}
			}
		}

		if ( bShowDebugLogs )
		{
			GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Red, FString::Printf( TEXT( "Accessible text: '%s'" ), *TextToSpeak.ToString() ) );
		}

		// Try to queue any new text that we have found
		bool bShouldProcess = true;
		if ( !TextToSpeak.EqualToCaseIgnored( LastTextWeSpoke ) )
		{
			// If we are mousing over something useful
			if ( !TextToSpeak.IsEmptyOrWhitespace() )
			{
				// TODO split by sentence etc.
				QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_SpeakText );

				TArray<FString> NewText;
				if ( SplitDelims.Num() > 0 )
				{
					TArray<FString> Split;
					TextToSpeak.ToString().ParseIntoArray( Split, SplitDelims.GetData(), SplitDelims.Num() );
					NewText.Append( Split );
				}
				else
				{
					NewText.Add( TextToSpeak.ToString() );
				}
				SpeakText( NewText );
			}

			// Save it before we split it up
			LastTextWeSpoke = TextToSpeak;
		}

		if ( !TextToSpeak.IsEmptyOrWhitespace() )
		{
			// We only process audio when we are mousing over something useful
			// Process any audio that has been returned
			{
				QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_GetAndClear );
				TArray<TUniquePtr<FBYGSoundWaveData>> NewSoundWaveData;
				TextToSpeechRunnable->GetAndClearSoundWaves( NewSoundWaveData );
				SoundWaveDataQueue.Append( MoveTemp( NewSoundWaveData ) );
				if ( bShowDebugLogs )
				{
					const FString DebugStr = FString::Printf( TEXT( "Sound wave queue: %d" ), SoundWaveDataQueue.Num() );
					GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Red, *DebugStr );
				}
			}

			if ( SoundWaveDataQueue.Num() > 0 && !bIsPlayingAudio )
			{
				QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_PlayAudio );
				// Wait until the current one is done
				USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>( this );
				SoundWave->NumChannels = 1;
				SoundWave->SetSampleRate( 44100.0 );
				SoundWave->Duration = ( SoundWaveDataQueue[ 0 ]->Buffer.GetAllocatedSize() / 2 ) / 44100.0;
				SoundWave->SoundGroup = SOUNDGROUP_Voice;
				SoundWave->bStreaming = false;
				DurationRemaining = SoundWave->Duration;

				// For some reason OnAudioFinished is never called when the procedurally-generated audio actually finishes.

				// This memcopies the buffer to be owned by the sound wave
				SoundWave->QueueAudio( SoundWaveDataQueue[ 0 ]->Buffer.GetData(), SoundWaveDataQueue[ 0 ]->Buffer.GetAllocatedSize() );

				// Deletes the memory owned by the TUniquePtr<T>
				SoundWaveDataQueue.RemoveAt( 0 );

				bIsPlayingAudio = true;
				if ( !AudioComponent )
				{
					// TODO reuse audo component?
					AudioComponent = UGameplayStatics::SpawnSound2D( GetWorld(), SoundWave );
					AudioComponent->bAutoDestroy = false;
					AudioComponent->OnAudioFinishedNative.AddUObject( this, &UBYGTextToSpeechSubsystem::OnAudioFinished );
				}
				else
				{
					AudioComponent->Stop();
					AudioComponent->SetSound( SoundWave );
					AudioComponent->Play();
				}
			}
		}
		else
		{
			StopAudio();
		}
	}
}

void GetChildrenAccessibleText( TSharedRef<SWidget> Owner, TArray<FString>& OutText, int32 Depth = 0 )
{
	const FText Text = Owner->GetAccessibleText( EAccessibleType::Summary );
	if ( !Text.IsEmpty() )
	{
		OutText.Add( Text.ToString() );
	}

	FChildren* AllChildren = Owner->GetAllChildren();
	if ( AllChildren )
	{
		for ( int32 i = 0; i < AllChildren->Num(); ++i )
		{
			TSharedRef<SWidget> Child = AllChildren->GetChildAt( i );
			GetChildrenAccessibleText( Child, OutText, Depth + 1 );
		}
	}
}

bool UBYGTextToSpeechSubsystem::SpeakText( const TArray<FString>& Text, bool bStopExisting )
{
	if ( !bIsEnabled )
	{
		if ( bShowDebugLogs )
		{
			GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Yellow, TEXT( "Tryng to speak text on disabled speech subsystem" ) );
		}
		return false;
	}
	
	if ( !TextToSpeechRunnable.IsValid() )
	{
		if ( bShowDebugLogs )
		{
			GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Red, TEXT( "Failed to speak text, TextToSpeechRunnable not initialized." ) );
		}
		return false;
	}

	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SpeakText );
	if ( bShowDebugLogs )
	{
		GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Green, FString::Printf( TEXT( "SpeakText: %d elements" ), Text.Num() ) );
	}
	TArray<FString> RequiredAttributes;
	RequiredAttributes.Add( FString::Printf( TEXT( "name=%s" ), *VoiceName ) );
	//RequiredAttributes.Add( FString::Printf( TEXT( "vendor=%s" ), *VoiceVendor ) );
	//RequiredAttributes.Add( FString::Printf( TEXT( "gender=%s" ), *VoiceGender ) );
	const int32 Rate = FMath::GetMappedRangeValueClamped( FVector2D( 0.0f, 1.0f ), FVector2D( -10, 10 ), Speed );
	const FString Attributes = FString::Join( RequiredAttributes, TEXT( ";" ) );

	// New text, so clear the queue
	if ( bStopExisting )
	{
		StopAudio();
	}
	TextToSpeechRunnable->SetRate( Rate );
	TextToSpeechRunnable->SetAttributes( Attributes );
	TextToSpeechRunnable->AddText( Text );

	return true;
}

bool UBYGTextToSpeechSubsystem::SetVoiceId( const FString& InVoiceId )
{
	const TArray<FBYGVoiceInfo> Voices = UBYGTextToSpeechStatics::GetVoices();
	const FBYGVoiceInfo* FoundInfo = Voices.FindByPredicate( [InVoiceId]( const FBYGVoiceInfo& Voice )
	{
		return Voice.Id == InVoiceId;
	} );
	if ( FoundInfo )
	{
		VoiceName = FoundInfo->Name;
		return true;
	}
	return false;
}

void UBYGTextToSpeechSubsystem::StopAudio()
{
	if ( AudioComponent )
	{
		AudioComponent->Stop();
	}

	SoundWaveDataQueue.Empty();
	if ( TextToSpeechRunnable.IsValid() )
	{
		TextToSpeechRunnable->ClearQueue();
	}
	DurationRemaining = INDEX_NONE;
}

void UBYGTextToSpeechSubsystem::SetIsEnabled( bool bInIsEnabled )
{
	if ( bInIsEnabled == bIsEnabled )
		return;

	bIsEnabled = bInIsEnabled;

	if (bIsEnabled)
	{
		CreateRunnable();
	}
	else
	{
		StopAudio();
		CleanUpRunnable();
	}
}

void FBYGTextToSpeechSubsystemTickFunction::ExecuteTick( float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent )
{
	Subsystem->Tick( DeltaTime, TickType, CurrentThread, MyCompletionGraphEvent );
}

FString FBYGTextToSpeechSubsystemTickFunction::DiagnosticMessage()
{
	static const FString Message( TEXT( "FBYGTextToSpeechSubsystemTickFunction" ) );
	return Message;
}

FName FBYGTextToSpeechSubsystemTickFunction::DiagnosticContext( bool bDetailed )
{
	static const FName Context( TEXT( "FBYGTextToSpeechSubsystemTickFunction" ) );
	return Context;
}

void UBYGTextToSpeechSubsystem::OnAudioFinished( UAudioComponent* AudioComp )
{
	if ( bShowDebugLogs )
	{
		GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Yellow, TEXT( "AUDIO FINISHED" ) );
	}
	bIsPlayingAudio = false;
}
