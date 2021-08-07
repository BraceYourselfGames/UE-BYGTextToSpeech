// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechRunnable.h"
#include "Async/Async.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Sound/SoundWaveProcedural.h"

#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#pragma warning(push)
#pragma warning(disable: 4191)
#pragma warning(disable: 4996)
#ifndef DeleteFile
#define DeleteFile DeleteFileW
#endif
#ifndef MoveFile
#define MoveFile MoveFileW
#endif
#ifndef LoadString
#define LoadString LoadStringW
#endif
#ifndef GetMessage
#define GetMessage GetMessageW
#endif

#include <atlbase.h>
#include <sapi.h>
#include <sphelper.h>

#undef DeleteFile
#undef MoveFile
#undef LoadString
#undef GetMessage
#pragma warning(pop)
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"

#endif

DEFINE_LOG_CATEGORY( LogBYGTextToSpeech );

HRESULT FBYGTextToSpeechRunnable::WaitAndPumpMessagesWithTimeout( HANDLE hWaitHandle, DWORD dwMilliseconds )
{
	HRESULT hr = S_OK;
	bool fContinue = true;

	bStopInner = false;

	while ( fContinue && !bStopInner )
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_WaitAndPump_Outer );

		DWORD dwWaitId = ::MsgWaitForMultipleObjectsEx( 1, &hWaitHandle, dwMilliseconds, QS_ALLINPUT, MWMO_INPUTAVAILABLE );
		switch ( dwWaitId )
		{
		case WAIT_OBJECT_0:
		{
			QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_WaitAndPump_Object0 );
			fContinue = false;
		}
		break;

		case WAIT_OBJECT_0 + 1:
		{
			MSG Msg;
			QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_WaitAndPump_Peek );
			while ( ::PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
			{
				QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_WaitAndPump_PeekInner );
				::TranslateMessage( &Msg );
				::DispatchMessage( &Msg );
			}
		}
		break;

		case WAIT_TIMEOUT:
		{
			QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_WaitAndPump_Timeout );
			hr = S_FALSE;
			fContinue = false;
		}
		break;

		default:// Unexpected error
		{
			QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_WaitAndPump_Error );
			fContinue = false;
			hr = E_FAIL;
		}
		break;
		}
	}
	return hr;
}

char* FBYGTextToSpeechRunnable::TextToWavInner( const wchar_t* voiceRequiredAttributes, const wchar_t* voiceOptionalAttributes, long rate, const wchar_t* textToRender, ULONG* pBytesRead )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner );

	HRESULT hr;
	CComPtr<ISpVoice> cpVoice; //Will send data to ISpStream
	CComPtr<ISpStream> cpStream; //Will contain IStream
	CComPtr<IStream> cpBaseStream; //raw data
	ISpObjectToken* cpToken( NULL ); //Will set voice characteristics

	GUID guidFormat;
	WAVEFORMATEX* pWavFormatEx = nullptr;

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_CreateVoiceInstance );
		hr = cpVoice.CoCreateInstance( CLSID_SpVoice );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_FindToken );
		hr = SpFindBestToken( SPCAT_VOICES, voiceRequiredAttributes, voiceOptionalAttributes, &cpToken );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_SetVoice );
		hr = cpVoice->SetVoice( cpToken );
		cpToken->Release();
		if ( FAILED( hr ) )
			return NULL;
	}

	cpVoice->SetRate( rate );

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_CreateStreamInstance );
		hr = cpStream.CoCreateInstance( CLSID_SpStream );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_CreateStream );
		hr = CreateStreamOnHGlobal( NULL, true, &cpBaseStream );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_ConvertStream );
		hr = SpConvertStreamFormatEnum( SPSF_44kHz16BitMono, &guidFormat, &pWavFormatEx );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_SetBaseStream );
		hr = cpStream->SetBaseStream( cpBaseStream, guidFormat, pWavFormatEx );
		if ( FAILED( hr ) )
			return NULL;
	}

	// Loads .dll here?
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_SetOutput );
		hr = cpVoice->SetOutput( cpStream, false );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_Speak );
		// This is the slow bit
		SpeechVoiceSpeakFlags voiceFlags = ( SpeechVoiceSpeakFlags ) ( SpeechVoiceSpeakFlags::SVSFlagsAsync );// | SpeechVoiceSpeakFlags::SVSFPurgeBeforeSpeak );
		hr = cpVoice->Speak( textToRender, voiceFlags, NULL );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		HANDLE hWait = cpVoice->SpeakCompleteEvent();
		WaitAndPumpMessagesWithTimeout( hWait, INFINITE );
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_Seek );
		LARGE_INTEGER a = { 0 };
		hr = cpStream->Seek( a, STREAM_SEEK_SET, NULL );
		if ( FAILED( hr ) )
			return NULL;
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInner_Read );
		STATSTG stats;
		cpStream->Stat( &stats, STATFLAG_NONAME );

		ULONG sSize = stats.cbSize.LowPart;

		char* pBuffer = new char[ sSize ];
		cpStream->Read( pBuffer, sSize, pBytesRead );
		return pBuffer;
	}

	return NULL;
}

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
				QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeech_SoundWaveInitialize );

				if ( SUCCEEDED( ::CoInitialize( NULL ) ) )
				{
					unsigned long BytesRead;
					FString VoiceOptionalAttributes = "";
					uint8* TTSAudioBuffer = ( uint8* ) TextToWavInner( *Attributes, *VoiceOptionalAttributes, Rate, *Text, &BytesRead );

					::CoUninitialize();

					if ( TTSAudioBuffer )
					{
						// We cannot create UObjects in a separate thread
						// So instead we return the raw data so it can be used to create objects
						// in the main thread
						FBYGSoundWaveData Data;
						Data.Buffer = TTSAudioBuffer;
						Data.BytesRead = BytesRead;

						{
							FScopeLock Lock( &SoundCriticalSection );
							SoundWaveData.Add( Data );
						}
					}
					else
					{
						UE_LOG( LogBYGTextToSpeech, Error,
							TEXT( "Can't generate wave from speech with required attributes '%s', optional attributes '%s'. Voice may not be found." ),
							*Attributes,
							*VoiceOptionalAttributes );
					}
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
	ClearQueue();
}

void FBYGTextToSpeechRunnable::Stop()
{
	bRunning = false;
	ClearQueue();
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
	bStopInner = true;
	{
		FScopeLock TextLock( &TextCriticalSection );
		TextQueue.Empty();
	}
	{
		FScopeLock SoundLock( &SoundCriticalSection );
		SoundWaveData.Empty();
	}
}

TArray<FBYGSoundWaveData> FBYGTextToSpeechRunnable::GetAndClearSoundWaves()
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGTextToSpeechRunnable_GetAndClear );
	FScopeLock Lock( &SoundCriticalSection );
	TArray<FBYGSoundWaveData> SoundWavesToReturn = SoundWaveData;
	SoundWaveData.Empty();
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

int32 FBYGTextToSpeechRunnable::GetTextQueueSize() const
{
	FScopeLock Lock( &TextCriticalSection );
	return TextQueue.Num();
}

int32 FBYGTextToSpeechRunnable::GetSoundWaveDataSize() const
{
	FScopeLock Lock( &SoundCriticalSection );
	return SoundWaveData.Num();
}
