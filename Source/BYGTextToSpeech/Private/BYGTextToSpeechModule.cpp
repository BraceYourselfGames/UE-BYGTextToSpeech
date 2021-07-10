// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechModule.h"
#include "BYGTextToSpeechStatics.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif


#define LOCTEXT_NAMESPACE "BYGTextToSpeechModule"

void FBYGTextToSpeechModule::StartupModule()
{
	LastFrameNumberWeTicked = INDEX_NONE;

	FDefaultGameModuleImpl::StartupModule();
}

void FBYGTextToSpeechModule::ShutdownModule()
{
	if ( AudioComponent )
	{
		AudioComponent->bAutoDestroy = true;
		AudioComponent = nullptr;
	}

	FDefaultGameModuleImpl::ShutdownModule();
}

void GetChildrenAccessibleText( TSharedRef<SWidget> Owner, TArray<FString>& OutText, int32 Depth = 0 )
{
	FString Indent = "";
	for (int32 i = 0; i < Depth; ++i )
	{
		Indent += "  ";
	}
	const FText Text = Owner->GetAccessibleText(EAccessibleType::Summary);
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

UWorld* FBYGTextToSpeechModule::GetWorld()
{
	UWorld* World = nullptr;

#if WITH_EDITOR
	UEditorEngine* EEngine = Cast<UEditorEngine>( GEngine );
	if ( GIsEditor && EEngine != nullptr )
	{
		// lets use PlayWorld during PIE/Simulate and regular world from editor otherwise, to draw debug information
		World = EEngine->PlayWorld != nullptr ? EEngine->PlayWorld : EEngine->GetEditorWorldContext().World();
	}
#endif

	if ( !GIsEditor && World == nullptr && GEngine != nullptr )
	{
		World = GEngine->GetWorld();
	}

	return World;
}

void FBYGTextToSpeechModule::Tick( float DeltaTime )
{
	if ( LastFrameNumberWeTicked != GFrameCounter )
	{
		if ( bAutoReadOnHover )
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
						RequiredAttributes.Add( FString::Printf( TEXT( "vendor=%s" ), *VoiceVendor ) );
						RequiredAttributes.Add( FString::Printf( TEXT( "gender=%s" ), *VoiceGender ) );
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

		LastFrameNumberWeTicked = GFrameCounter;
	}
}

void FBYGTextToSpeechModule::SetVoiceId( const FString& InVoiceId )
{
	TArray<FBYGVoiceInfo> Voices = UBYGTextToSpeechStatics::GetVoices();
	FBYGVoiceInfo* FoundInfo = Voices.FindByPredicate( [InVoiceId]( const FBYGVoiceInfo& Voice )
	{
		return Voice.Id == InVoiceId;
	} );
	if ( FoundInfo )
	{
		VoiceName = FoundInfo->Name;
		VoiceVendor = FoundInfo->Vendor;
		VoiceGender = FoundInfo->Gender == EBYGSpeakerGender::Masculine ? "male" : "female";
	}
}

void FBYGTextToSpeechModule::SetIsEnabled( bool bInIsEnabled )
{
	if ( bInIsEnabled == bIsEnabled)
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

void FBYGTextToSpeechModule::AddReferencedObjects( FReferenceCollector& Collector )
{
	Collector.AddReferencedObject( AudioComponent );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGTextToSpeechModule, BYGTextToSpeech );


