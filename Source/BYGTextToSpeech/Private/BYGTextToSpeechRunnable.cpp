// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "Async/Async.h"
#include "BYGTextToSpeechRunnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Sound/SoundWave.h"
#include "BYGTextToSpeechStatics.h"
#include <Kismet/GameplayStatics.h>


FBYGTextToSpeechRunnable::FBYGTextToSpeechRunnable()
	: bRunning( true )
	, Thread( nullptr )
{
	Thread = FRunnableThread::Create( this, TEXT( "BYGTextToSpeechRunnable" ) ); // , 8 * 1024, TPri_Normal );
}

FBYGTextToSpeechRunnable::~FBYGTextToSpeechRunnable()
{
	Stop();

	if ( Thread != nullptr )
	{
		Thread->Kill( true );
		delete Thread;
	}
}

uint32 FBYGTextToSpeechRunnable::Run()
{
	while ( bRunning )
	{
		{
			FScopeLock Lock( &TextCriticalSection );

			if ( TextQueue.Num() > 0 )
			{
				const FString Text = TextQueue[ 0 ];
				TextQueue.RemoveAt( 0 );

				USoundWave* SoundWave = UBYGTextToSpeechStatics::TextToSoundWaveAdvanced(
					Attributes,
					"",
					Rate,
					Text
				);

				//OnTTSCompleteDelegate.ExecuteIfBound( SoundWave );
				//AudioComponent = UGameplayStatics::SpawnSound2D( GetWorld(), SoundWave );
				//UGameplayStatics::SpawnSound2D( GetWorld(), SoundWave );

				SoundWaves.Add( SoundWave );
			}
		}

		//FPlatformProcess::Sleep( 0.1f );
	}

	// Return success
	return 0;
}

void FBYGTextToSpeechRunnable::Exit()
{
	// Here's where we can do any cleanup we want to 
	bRunning = false;
}

void FBYGTextToSpeechRunnable::Stop()
{
	bRunning = false;
}


void FBYGTextToSpeechRunnable::AddText( const FString& Text )
{
	FScopeLock Lock( &TextCriticalSection );
	TextQueue.Add( Text );
}

void FBYGTextToSpeechRunnable::AddText( const TArray<FString>& Text )
{
	FScopeLock Lock( &TextCriticalSection );
	TextQueue.Append( Text );
}

void FBYGTextToSpeechRunnable::ClearQueue()
{
	FScopeLock Lock( &TextCriticalSection );
	TextQueue.Empty();
	SoundWaves.Empty();
}

TArray<USoundWave*> FBYGTextToSpeechRunnable::GetAndClearSoundWaves()
{
	FScopeLock Lock( &TextCriticalSection );
	// uhh
	TArray<USoundWave*> SoundWavesToReturn = SoundWaves;
	SoundWaves.Empty();
	return SoundWavesToReturn;
}

void FBYGTextToSpeechRunnable::SetRate( int32 InRate )
{
	Rate = InRate;
}

void FBYGTextToSpeechRunnable::SetAttributes( const FString& InAttributes )
{
	Attributes = InAttributes;
}

