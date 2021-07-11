#include "BYGTextToSpeechSoundWave.h"

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

char* TextToWavInner( const wchar_t* voiceRequiredAttributes, const wchar_t* voiceOptionalAttributes, long rate, const wchar_t* textToRender, ULONG* pBytesRead )
{
	HRESULT hr;
	CComPtr<ISpVoice> cpVoice; //Will send data to ISpStream
	CComPtr<ISpStream> cpStream; //Will contain IStream
	CComPtr<IStream> cpBaseStream; //raw data
	ISpObjectToken* cpToken( NULL ); //Will set voice characteristics

	GUID guidFormat;
	WAVEFORMATEX* pWavFormatEx = nullptr;

	hr = cpVoice.CoCreateInstance( CLSID_SpVoice );
	if ( FAILED( hr ) )
		return NULL;

	hr = SpFindBestToken( SPCAT_VOICES, voiceRequiredAttributes, voiceOptionalAttributes, &cpToken );
	if ( FAILED( hr ) )
		return NULL;

	hr = cpVoice->SetVoice( cpToken );
	cpToken->Release();
	if ( FAILED( hr ) )
		return NULL;

	cpVoice->SetRate( rate );

	hr = cpStream.CoCreateInstance( CLSID_SpStream );
	if ( FAILED( hr ) )
		return NULL;

	hr = CreateStreamOnHGlobal( NULL, true, &cpBaseStream );
	if ( FAILED( hr ) )
		return NULL;

	hr = SpConvertStreamFormatEnum( SPSF_44kHz16BitMono, &guidFormat, &pWavFormatEx );
	if ( FAILED( hr ) )
		return NULL;

	hr = cpStream->SetBaseStream( cpBaseStream, guidFormat, pWavFormatEx );
	if ( FAILED( hr ) )
		return NULL;

	hr = cpVoice->SetOutput( cpStream, false );
	if ( FAILED( hr ) )
		return NULL;

	SpeechVoiceSpeakFlags voiceFlags = SpeechVoiceSpeakFlags::SVSFDefault;
	hr = cpVoice->Speak( textToRender, voiceFlags, NULL );
	if ( FAILED( hr ) )
		return NULL;

	// Uncomment below to directly output speech
	//cpVoice->SetOutput(NULL, FALSE);
	//cpVoice->SpeakStream(cpStream, SPF_DEFAULT, NULL);

	LARGE_INTEGER a = { 0 };
	hr = cpStream->Seek( a, STREAM_SEEK_SET, NULL );
	if ( FAILED( hr ) )
		return NULL;

	STATSTG stats;
	cpStream->Stat( &stats, STATFLAG_NONAME );

	ULONG sSize = stats.cbSize.LowPart;

	char* pBuffer = new char[ sSize ];
	cpStream->Read( pBuffer, sSize, pBytesRead );

	return pBuffer;
}

bool UBYGTextToSpeechSoundWave::Initialize(const FString &VoiceRequiredAttributes, const FString &VoiceOptionalAttributes, int32 Rate, const FString &Text)
{
	if ( Text.IsEmpty() )
		return false;

	if ( FAILED( ::CoInitialize( NULL ) ) )
		return false;

	unsigned long BytesRead;
	uint8 *TTSAudioBuffer = (uint8*)TextToWavInner(*VoiceRequiredAttributes, *VoiceOptionalAttributes, Rate, *Text, &BytesRead);

	::CoUninitialize();

	if ( TTSAudioBuffer )
	{
		NumChannels = 1;
		SampleRate = 44100;
		Duration = (BytesRead / 2) / 44100.0;
		SoundGroup = SOUNDGROUP_Voice;
		this->bStreaming = false;
		
		//TotalSamples = SamplesRead / 1000.0;
		//ChannelSizes.Add(BytesRead);

		QueueAudio((uint8*)TTSAudioBuffer, BytesRead);
		
		delete TTSAudioBuffer;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Can't generate wave from speech (may be voice not found, check VoiceRequiredAttributes)."));
	}

	return true;
}


