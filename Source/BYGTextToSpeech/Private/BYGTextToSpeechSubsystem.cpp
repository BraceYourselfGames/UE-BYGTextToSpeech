// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "BYGTextToSpeechStatics.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "BYGTextToSpeechRunnable.h"

//DECLARE_CYCLE_STAT(TEXT("BYGTextToSpeechSubsystem Tick"), STAT_BYGTextToSpeechSubsystemTick, STATGROUP_BYGTextToSpeech);

#define BYG_TTS_VISUAL_LOG true

UBYGTextToSpeechSubsystem::UBYGTextToSpeechSubsystem()
	: TickFunction( this )
{

	SplitDelims = {
		TEXT( "." ),
		TEXT( "\r\n" ),
		TEXT( "\n" )
	};
}

UBYGTextToSpeechSubsystem::~UBYGTextToSpeechSubsystem()
{
	if ( TextToSpeechRunnable.IsValid() )
	{
		TextToSpeechRunnable->Exit();
		TextToSpeechRunnable.Reset();
	}
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
	// Register Tick 
	TickFunction.bCanEverTick = true;
	TickFunction.bTickEvenWhenPaused = true;
	TickFunction.bStartWithTickEnabled = true;
	TickFunction.TickGroup = TG_DuringPhysics;
	TickFunction.bAllowTickOnDedicatedServer = false;
	TickFunction.RegisterTickFunction( GetWorld()->PersistentLevel );
}

void UBYGTextToSpeechSubsystem::Deinitialize()
{
	if ( TextToSpeechRunnable.IsValid() )
	{
		TextToSpeechRunnable->Exit();
		TextToSpeechRunnable.Reset();
	}

	TickFunction.UnRegisterTickFunction();
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

	if ( !TextToSpeechRunnable.IsValid() )
	{
		TextToSpeechRunnable = MakeUnique<FBYGTextToSpeechRunnable>();
	}

	if ( VoiceName.IsEmpty() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Text to speech subsystem is enabled, but no voice ID set" ) );
		return;
	}

	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_IfIsEnabled );
	// Find the widget that we are over, and any text widgets that beneath it
	const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
	const FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse( CursorPos, FSlateApplication::Get().GetInteractiveTopLevelWindows() );


#if BYG_TTS_VISUAL_LOG
	GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Green, FString::Printf( TEXT( "Runnable Data: %d" ), TextToSpeechRunnable->GetSoundWaveDataSize() ) );
	GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Magenta, FString::Printf( TEXT( "Runnable Text: %d" ), TextToSpeechRunnable->GetTextQueueSize() ) );
#endif

	static const FString RootName = "SGameLayerManager";

	// Anything inside here, and we're hovering over one of our widgets
	TArray<FString> AllText;
	if ( WidgetPath.IsValid() )
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_AllText );
#if BYG_TTS_VISUAL_LOG
		FString Indent = "";
#endif
		for ( int32 i = 0; i < WidgetPath.Widgets.Num(); ++i )
		{
			const FArrangedWidget& ArrangedWidget = WidgetPath.Widgets[ i ];
			const TSharedRef<SWidget> Widget = ArrangedWidget.Widget;

#if BYG_TTS_VISUAL_LOG
			Indent += " ";
			FString DebugStr = FString::Printf( TEXT( "%s%s" ), *Indent, *Widget->ToString() );
			//GEngine->AddOnScreenDebugMessage( i, 0, FColor::Green, *DebugStr );
#endif

			if ( Widget->GetTypeAsString() == RootName )
			{
				GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Green, "FOUND!" );
				TSharedRef<SWidget> LeafWidget = WidgetPath.GetLastWidget();
				const FText Text = LeafWidget->GetAccessibleText( EAccessibleType::Summary );
				if ( !Text.IsEmptyOrWhitespace() )
				{
					AllText.Add( Text.ToString() );
				}
				//GetChildrenAccessibleText( LeafWidget, AllText );
				break;
			}
		}
	}

#if BYG_TTS_VISUAL_LOG
	{
		const FString DebugStr = FString::Join( AllText, TEXT( "\n" ) );
		GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Red, DebugStr );
	}
#endif

	// Try to queue any new text that we have found
	if ( AllText != LastTextWeSpoke )
	{
		// Save it before we split it up
		LastTextWeSpoke = AllText;
		if ( AllText.Num() > 0 )
		{
			// TODO split by sentence etc.
			QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_SpeakText );

			if ( SplitDelims.Num() > 0 )
			{
				TArray<FString> NewText;
				for ( const FString Text : AllText )
				{
					TArray<FString> Split;
					Text.ParseIntoArray( Split, SplitDelims.GetData(), SplitDelims.Num() );
					NewText.Append( Split );
				}
				AllText = NewText;
			}

			SpeakText( AllText );
		}
		else
		{
			StopAudio();
		}
	}

	// Process any audio that has been returned
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_GetAndClear );
		TArray<FBYGSoundWaveData> NewSoundWaveData = TextToSpeechRunnable->GetAndClearSoundWaves();
		SoundWaveDataQueue.Append( NewSoundWaveData );
#if BYG_TTS_VISUAL_LOG
		const FString DebugStr = FString::Printf( TEXT( "Sound wave queue: %d" ), SoundWaveDataQueue.Num() );
		GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Red, *DebugStr );
#endif
	}

	if ( SoundWaveDataQueue.Num() > 0 && !bIsPlayingAudio )
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechSubsystemTick_PlayAudio );
		// Wait until the current one is done
		USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>( this );
		SoundWave->NumChannels = 1;
		SoundWave->SetSampleRate( 44100.0 );
		SoundWave->Duration = ( SoundWaveDataQueue[ 0 ].BytesRead / 2 ) / 44100.0;
		SoundWave->SoundGroup = SOUNDGROUP_Voice;
		SoundWave->bStreaming = false;

		// This memcopies the buffer to be owned by the sound wave
		SoundWave->QueueAudio( ( uint8* ) SoundWaveDataQueue[ 0 ].Buffer, SoundWaveDataQueue[ 0 ].BytesRead );

		// TODO Memory is almost certainly leaking
		delete SoundWaveDataQueue[ 0 ].Buffer;

		bIsPlayingAudio = true;
		if ( !AudioComponent )
		{
			// TODO reuse audo component?
			AudioComponent = UGameplayStatics::SpawnSound2D( GetWorld(), SoundWave );
			AudioComponent->bAutoDestroy = false;
			AudioComponent->OnAudioFinished.AddUniqueDynamic( this, &UBYGTextToSpeechSubsystem::OnAudioFinishedDynamic );
			AudioComponent->OnAudioFinishedNative.AddUObject( this, &UBYGTextToSpeechSubsystem::OnAudioFinished );
		}
		else if ( bIsPlayingAudio ) //!AudioComponent->IsPlaying() )
		{
			AudioComponent->Stop();
			AudioComponent->SetSound( SoundWave );
			AudioComponent->Play();
		}
		SoundWaveDataQueue.RemoveAt( 0 );
	}
}

void GetChildrenAccessibleText( TSharedRef<SWidget> Owner, TArray<FString>& OutText, int32 Depth = 0 )
{
#if BYG_TTS_VISUAL_LOG
	FString Indent = "";
	for ( int32 i = 0; i < Depth; ++i )
	{
		Indent += "  ";
	}
#endif
	const FText Text = Owner->GetAccessibleText( EAccessibleType::Summary );
	if ( !Text.IsEmpty() )
	{
		OutText.Add( Text.ToString() );
	}

#if BYG_TTS_VISUAL_LOG
	const FString DebugStr = FString::Printf( TEXT( "%s%s" ), *Indent, *Owner->ToString() );
	GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Red, *DebugStr );
#endif
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

void UBYGTextToSpeechSubsystem::SpeakText( const TArray<FString>& Text )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SpeakText );
#if BYG_TTS_VISUAL_LOG
	GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Green, FString::Printf( TEXT( "SpeakText: %d elements" ), Text.Num() ) );
#endif
	TArray<FString> RequiredAttributes;
	RequiredAttributes.Add( FString::Printf( TEXT( "name=%s" ), *VoiceName ) );
	//RequiredAttributes.Add( FString::Printf( TEXT( "vendor=%s" ), *VoiceVendor ) );
	//RequiredAttributes.Add( FString::Printf( TEXT( "gender=%s" ), *VoiceGender ) );
	const int32 Rate = FMath::GetMappedRangeValueClamped( FVector2D( 0.0f, 1.0f ), FVector2D( -10, 10 ), Speed );
	const FString Attributes = FString::Join( RequiredAttributes, TEXT( ";" ) );

	StopAudio();
	// New text, so clear the queue
	TextToSpeechRunnable->SetRate( Rate );
	TextToSpeechRunnable->SetAttributes( Attributes );
	TextToSpeechRunnable->ClearQueue();
	TextToSpeechRunnable->AddText( Text );
}

bool UBYGTextToSpeechSubsystem::SetVoiceId( const FString& InVoiceId )
{
	TArray<FBYGVoiceInfo> Voices = UBYGTextToSpeechStatics::GetVoices();
	FBYGVoiceInfo* FoundInfo = Voices.FindByPredicate( [InVoiceId]( const FBYGVoiceInfo& Voice )
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

void UBYGTextToSpeechSubsystem::ReadTextUnderCursor()
{

}

void UBYGTextToSpeechSubsystem::StopAudio()
{
	if ( AudioComponent )
	{
		AudioComponent->Stop();
		AudioComponent = nullptr;

	}
	SoundWaveDataQueue.Empty();
	if ( TextToSpeechRunnable.IsValid() )
	{
		TextToSpeechRunnable->GetAndClearSoundWaves();
		TextToSpeechRunnable->ClearQueue();
	}
}

void UBYGTextToSpeechSubsystem::SetIsEnabled( bool bInIsEnabled )
{
	if ( bInIsEnabled == bIsEnabled )
		return;

	bIsEnabled = bInIsEnabled;

	if ( !bIsEnabled )
	{
		StopAudio();

		TextToSpeechRunnable.Reset();
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

void UBYGTextToSpeechSubsystem::OnAudioFinishedDynamic()
{
#if BYG_TTS_VISUAL_LOG
	GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Yellow, TEXT( "AUDIO FINISHED" ) );
#endif
	bIsPlayingAudio = false;
}
void UBYGTextToSpeechSubsystem::OnAudioFinished( UAudioComponent* AudioComp )
{
#if BYG_TTS_VISUAL_LOG
	GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Yellow, TEXT( "AUDIO FINISHED" ) );
#endif
	bIsPlayingAudio = false;
}

