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
	Thread = FRunnableThread::Create( this, TEXT( "BYGTextToSpeechRunnable" ) );
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
			FString Text = "";
			{
				FScopeLock Lock( &TextCriticalSection );
				if ( TextQueue.Num() > 0 )
				{
					QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SpeakText );
					Text = TextQueue[ 0 ];
					TextQueue.RemoveAt( 0 );
				}
			}

			if ( !Text.IsEmpty() )
			{
				USoundWave* SoundWave = UBYGTextToSpeechStatics::TextToSoundWaveAdvanced(
					Attributes,
					"",
					Rate,
					Text
				);

				{
					FScopeLock Lock( &SoundCriticalSection );
					SoundWaves.Add( SoundWave );
				}
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
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechRunnable_AddText );
	FScopeLock Lock( &TextCriticalSection );
	TextQueue.Add( Text );
}

void FBYGTextToSpeechRunnable::AddText( const TArray<FString>& Text )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechRunnable_AddTextArr );
	FScopeLock Lock( &TextCriticalSection );
	TextQueue.Append( Text );
}

void FBYGTextToSpeechRunnable::ClearQueue()
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechRunnable_ClearQueue );
	//Stop();

	// Does this kill the thread in a sensible way?
	//if ( Thread != nullptr )
	//{
		//Thread->Kill( true );
		//delete Thread;
		//Thread = nullptr;
	//}
	//Thread = FRunnableThread::Create( this, TEXT( "BYGTextToSpeechRunnable" ) ); // , 8 * 1024, TPri_Normal );
	//bRunning = true;

	// TODO this will block...
	FScopeLock TextLock( &TextCriticalSection );
	FScopeLock SoundLock( &SoundCriticalSection );
	TextQueue.Empty();
	SoundWaves.Empty();
}

TArray<USoundWave*> FBYGTextToSpeechRunnable::GetAndClearSoundWaves()
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechRunnable_GetAndClear );
	FScopeLock Lock( &SoundCriticalSection );
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

