// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechRunnable.h"
#include "BYGTextToSpeechLog.h"
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

//FBYGSoundWaveData::~FBYGSoundWaveData()
//{
	//delete[] Buffer;
//}

HRESULT FBYGTextToSpeechRunnable::WaitAndPumpMessagesWithTimeout( HANDLE hWaitHandle, DWORD dwMilliseconds )
{
	HRESULT hr = S_OK;

	bool bSucceeded = false;
	bCancelInnerLoop = false;

	while ( !bCancelInnerLoop && !bSucceeded )
	{
		DWORD dwWaitId = ::MsgWaitForMultipleObjectsEx( 1, &hWaitHandle, dwMilliseconds, QS_ALLINPUT, MWMO_INPUTAVAILABLE );
		switch ( dwWaitId )
		{
		case WAIT_OBJECT_0:
		{
			bSucceeded = true;
		}
		break;

		case WAIT_OBJECT_0 + 1:
		{
			MSG Msg;
			while ( ::PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
			{
				::TranslateMessage( &Msg );
				::DispatchMessage( &Msg );
			}
		}
		break;

		case WAIT_TIMEOUT:
		{
			bCancelInnerLoop = true;
			hr = S_FALSE;
		}
		break;

		default:// Unexpected error
		{
			bCancelInnerLoop = true;
			hr = E_FAIL;
		}
		break;
		}
	}
	return hr;
}

bool FBYGTextToSpeechRunnable::TextToWavInner( const wchar_t* voiceRequiredAttributes, const wchar_t* voiceOptionalAttributes, long rate, const wchar_t* textToRender, TArray<uint8>& Buffer )
{
	HRESULT hr;
	CComPtr<ISpVoice> cpVoice; //Will send data to ISpStream
	CComPtr<ISpStream> cpStream; //Will contain IStream
	CComPtr<IStream> cpBaseStream; //raw data
	ISpObjectToken* cpToken( NULL ); //Will set voice characteristics

	GUID guidFormat;
	WAVEFORMATEX* pWavFormatEx = nullptr;

	{
		hr = cpVoice.CoCreateInstance( CLSID_SpVoice );
		if ( FAILED( hr ) )
			return false;
	}

	{
		hr = SpFindBestToken( SPCAT_VOICES, voiceRequiredAttributes, voiceOptionalAttributes, &cpToken );
		if ( FAILED( hr ) )
			return false;
	}

	{
		hr = cpVoice->SetVoice( cpToken );
		cpToken->Release();
		if ( FAILED( hr ) )
			return false;
	}

	cpVoice->SetRate( rate );

	{
		hr = cpStream.CoCreateInstance( CLSID_SpStream );
		if ( FAILED( hr ) )
			return false;
	}

	{
		hr = CreateStreamOnHGlobal( NULL, true, &cpBaseStream );
		if ( FAILED( hr ) )
			return false;
	}

	{
		hr = SpConvertStreamFormatEnum( SPSF_44kHz16BitMono, &guidFormat, &pWavFormatEx );
		if ( FAILED( hr ) )
			return false;
	}

	{
		hr = cpStream->SetBaseStream( cpBaseStream, guidFormat, pWavFormatEx );
		if ( FAILED( hr ) )
			return false;
	}

	// Loads .dll here?
	{
		hr = cpVoice->SetOutput( cpStream, false );
		if ( FAILED( hr ) )
			return false;
	}

	{
		// This is the slow bit
		SpeechVoiceSpeakFlags voiceFlags = ( SpeechVoiceSpeakFlags ) ( SpeechVoiceSpeakFlags::SVSFlagsAsync );// | SpeechVoiceSpeakFlags::SVSFPurgeBeforeSpeak );
		hr = cpVoice->Speak( textToRender, voiceFlags, NULL );
		if ( FAILED( hr ) )
			return false;
	}

	{
		HANDLE hWait = cpVoice->SpeakCompleteEvent();
		WaitAndPumpMessagesWithTimeout( hWait, INFINITE );
	}

	{
		LARGE_INTEGER a = { 0 };
		hr = cpStream->Seek( a, STREAM_SEEK_SET, NULL );
		if ( FAILED( hr ) )
			return false;
	}

	{
		STATSTG stats;
		cpStream->Stat( &stats, STATFLAG_NONAME );

		ULONG sSize = stats.cbSize.LowPart;

		// ALLOC MEMORY HERE
		//char* pBuffer = new char[ sSize ];
		//TUniquePtr<uint8> pBuffer = TUniquePtr<uint8>( new uint8[ sSize ] );
		Buffer.SetNumUninitialized( sSize );
		ULONG* pBytesRead = nullptr;
		cpStream->Read( Buffer.GetData(), sSize, pBytesRead );
		//ensure( pBytesRead != nullptr && sSize == *pBytesRead );
		return true;
	}

	return false;
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
					Text = TextQueue[ 0 ];
					TextQueue.RemoveAt( 0 );
				}
			}

			if ( !Text.IsEmpty() )
			{
				if ( SUCCEEDED( ::CoInitialize( NULL ) ) )
				{
					///unsigned long BytesRead;
					FString VoiceOptionalAttributes = "";
					//uint8* TTSAudioBuffer = ( uint8* ) TextToWavInner( *Attributes, *VoiceOptionalAttributes, Rate, *Text, &BytesRead );
					FBYGSoundWaveData Data;
					//TArray<uint8> TTSAudioBuffer;
					const bool bSuccess = TextToWavInner( *Attributes, *VoiceOptionalAttributes, Rate, *Text, Data.Buffer );

					::CoUninitialize();

					if ( bSuccess )
					{
						// We cannot create UObjects in a separate thread
						// So instead we return the raw data so it can be used to create objects
						// in the main thread
						//TUniquePtr<FBYGSoundWaveData> Data = MakeUnique<FBYGSoundWaveData>( MoveTemp( TTSAudioBuffer ), BytesRead );

						// There's a chance that we were told to cancel from an external source
						// In that case we don't want to the now-returned data to be added
						if ( !bCancelInnerLoop )
						{
							FScopeLock Lock( &SoundCriticalSection );
							// TODO// Copying the entire damn data of the buffer?????
							SoundWaveData.Add( MakeUnique<FBYGSoundWaveData>( Data ) );
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
	bCancelInnerLoop = true;
	{
		FScopeLock TextLock( &TextCriticalSection );
		TextQueue.Empty();
	}
	{
		FScopeLock SoundLock( &SoundCriticalSection );
		SoundWaveData.Empty();
	}
}

void FBYGTextToSpeechRunnable::GetAndClearSoundWaves( TArray<TUniquePtr<FBYGSoundWaveData>>& OutData )
{
	FScopeLock Lock( &SoundCriticalSection );
	// TArray<TUniquePtr<FBYGSoundWaveData>> SoundWavesToReturn = MoveTemp( SoundWaveData );
	OutData.Append( MoveTemp( SoundWaveData ) );
	SoundWaveData.Empty();
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
