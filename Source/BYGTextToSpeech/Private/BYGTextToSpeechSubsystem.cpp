// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "BYGTextToSpeechStatics.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "ProfilingDebugging/CsvProfiler.h"

//DECLARE_CYCLE_STAT(TEXT("BYGTextToSpeechSubsystem Tick"), STAT_BYGTextToSpeechSubsystemTick, STATGROUP_BYGTextToSpeech);

UBYGTextToSpeechSubsystem::UBYGTextToSpeechSubsystem()
	: TickFunction( this )
{
	LastFrameNumberWeTicked = INDEX_NONE;
}

UBYGTextToSpeechSubsystem::~UBYGTextToSpeechSubsystem()
{
}

void UBYGTextToSpeechSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Register Tick 
	TickFunction.bCanEverTick = true;
	TickFunction.bTickEvenWhenPaused = true;
	TickFunction.bStartWithTickEnabled = true;
	TickFunction.TickGroup = TG_DuringPhysics;
	TickFunction.bAllowTickOnDedicatedServer = false;
	TickFunction.RegisterTickFunction(GetWorld()->PersistentLevel);
}

void UBYGTextToSpeechSubsystem::Deinitialize()
{
	TickFunction.UnRegisterTickFunction();
}

void UBYGTextToSpeechSubsystem::Tick( float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent )
{
	//SCOPE_CYCLE_COUNTER(STAT_BYGTextToSpeechSubsystemTick);
	//TRACE_CPUPROFILER_EVENT_SCOPE(UBYGTextToSpeechWorldSubsystem::Tick);
	//CSV_SCOPED_TIMING_STAT_EXCLUSIVE(BYGTextToSpeech);
	//LLM_SCOPE(ELLMTag::BYGTextToSpeech);

	// Don't want to tick in-editor
	if ( LastFrameNumberWeTicked != GFrameCounter )
	{
		if ( bIsEnabled )
		{
			if ( VoiceName.IsEmpty() )
			{
				UE_LOG( LogTemp, Warning, TEXT( "Text to speech subsystem is enabled, but no voice ID set" ) );
			}
			else
			{
				// Find the widget that we are over, and any text widgets that beneath
				// it
				const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
				const FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse( CursorPos, FSlateApplication::Get().GetInteractiveTopLevelWindows() );

				static const FString RootName = "SGameLayerManager";

				// Anything inside here, and we're hovering over one of our widgets
				TArray<FString> AllText;
				if ( WidgetPath.IsValid() )
				{
					FString Indent = "";
					for ( int32 i = 0; i < WidgetPath.Widgets.Num(); ++i )
					{
						const FArrangedWidget& ArrangedWidget = WidgetPath.Widgets[ i ];
						const TSharedRef<SWidget> Widget = ArrangedWidget.Widget;

						Indent += " ";
						FString DebugStr = FString::Printf( TEXT( "%s%s" ), *Indent, *Widget->ToString() );
						GEngine->AddOnScreenDebugMessage( i, 0, FColor::Green, *DebugStr );

						if ( Widget->GetTypeAsString() == RootName )
						{
							GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Green, "FOUND!" );
							TSharedRef<SWidget> LeafWidget = WidgetPath.GetLastWidget();
							const FText Text = LeafWidget->GetAccessibleText( EAccessibleType::Summary );
							AllText.Add( Text.ToString() );
							//GetChildrenAccessibleText( LeafWidget, AllText );
							break;
						}
					}
				}

				const FString DebugStr = FString::Join( AllText, TEXT( "\n" ) );
				GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Red, DebugStr );

				if ( AllText != LastTextWeSpoke )
				{
					if ( AllText.Num() > 0 )
					{
						const FString OutString = FString::Join( AllText, TEXT( "\n" ) );

						if ( !OutString.IsEmpty() )
						{
							TArray<FString> RequiredAttributes;
							RequiredAttributes.Add( FString::Printf( TEXT( "name=%s" ), *VoiceName ) );
							//RequiredAttributes.Add( FString::Printf( TEXT( "vendor=%s" ), *VoiceVendor ) );
							//RequiredAttributes.Add( FString::Printf( TEXT( "gender=%s" ), *VoiceGender ) );
							const int32 Rate = FMath::GetMappedRangeValueClamped( FVector2D( 0.0f, 1.0f ), FVector2D( -10, 10 ), Speed );
							const FString Attributes = FString::Join( RequiredAttributes, TEXT( ";" ) );
							USoundWave* SoundWave = UBYGTextToSpeechStatics::TextToSoundWaveAdvanced(
								Attributes,
								"",
								Rate,
								OutString
							);

#if 0
							if ( !AudioComponent )
							{
								FAudioDevice::FCreateComponentParams Params = FAudioDevice::FCreateComponentParams( GetWorld()->GetAudioDeviceRaw() );
								AudioComponent = FAudioDevice::CreateComponent( SoundWave, Params );
								if ( AudioComponent )
								{
									AudioComponent->SetVolumeMultiplier( VolumeMultiplier );
									AudioComponent->SetPitchMultiplier( 1.0f );
									AudioComponent->bAllowSpatialization = false;
									AudioComponent->bIsUISound = true;
									AudioComponent->bAutoDestroy = false;
									AudioComponent->bIgnoreForFlushing = true;
									//AudioComponent->SubtitlePriority = Sound->GetSubtitlePriority();
								}
							}
							else
							{
								AudioComponent->Stop();
								AudioComponent->SetSound( SoundWave );
								AudioComponent->Play();
							}
#endif

							if ( AudioComponent )
								AudioComponent->Stop();

							AudioComponent = UGameplayStatics::SpawnSound2D( GetWorld(), SoundWave );
						}
					}
					LastTextWeSpoke = AllText;
				}
			}
		}

		LastFrameNumberWeTicked = GFrameCounter;
	}
}

void GetChildrenAccessibleText( TSharedRef<SWidget> Owner, TArray<FString>& OutText, int32 Depth = 0 )
{
	FString Indent = "";
	for ( int32 i = 0; i < Depth; ++i )
	{
		Indent += "  ";
	}
	const FText Text = Owner->GetAccessibleText( EAccessibleType::Summary );
	if ( !Text.IsEmpty() )
	{
		OutText.Add( Text.ToString() );
	}

	const FString DebugStr = FString::Printf( TEXT( "%s%s" ), *Indent, *Owner->ToString() );
	GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Red, *DebugStr );
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
	}
}

void UBYGTextToSpeechSubsystem::SetIsEnabled( bool bInIsEnabled )
{
	if ( bInIsEnabled == bIsEnabled )
		return;

	bIsEnabled = bInIsEnabled;

	if ( !bIsEnabled )
	{
		if ( AudioComponent )
		{
			AudioComponent->Stop();
			AudioComponent = nullptr;
		}
	}
	else
	{

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
