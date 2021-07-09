#include "BYGTextToSpeechStatics.h"
#include "BYGTextToSpeechSoundWave.h"
#if PLATFORM_WINDOWS
//#include "Windows/MinWindows.h"

#if 0

#include "CoreTypes.h"
#include "HAL/PlatformMemory.h"
#include "Windows/PreWindowsApi.h"
#ifndef STRICT
#define STRICT
#endif
#include "Windows/MinWindows.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"

#pragma warning(push)
#pragma warning(disable: 4191) // warning C4191: 'type cast' : unsafe conversion
#pragma warning(disable: 4996) // error C4996: 'GetVersionEx': was declared deprecated

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// atltransactionmanager.h doesn't use the W equivalent functions, use this workaround
#ifndef DeleteFile
#define DeleteFile DeleteFileW
#endif
#ifndef MoveFile
#define MoveFile MoveFileW
#endif

#include <atlbase.h>
#include <winnls.h>
#include <sapi.h>

#undef DeleteFile
#undef MoveFile

#include <sphelper.h>

#pragma warning(pop)


#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/PostWindowsApi.h"

#else

#include "Windows/AllowWindowsPlatformTypes.h"
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
#ifndef InterlockedDecrement
#define InterlockedDecrement _InterlockedDecrement
#endif
#ifndef InterlockedIncrement
#define InterlockedIncrement _InterlockedIncrement
#endif
#ifndef GetMessage
#define GetMessage GetMessageW
#endif

#include "sapi.h"
#include "sphelper.h"

#undef DeleteFile
#undef MoveFile
#undef LoadString
#undef InterlockedDecrement
#undef InterlockedIncrement
#undef GetMessage
#pragma warning(pop)
#include "Windows/HideWindowsPlatformTypes.h"

#endif

#endif

USoundWave* UBYGTextToSpeechStatics::TextToWave( FString VoiceRequiredAttributes, FString VoiceOptionalAttributes, int32 Rate, FString Text )
{
	auto TTSSoundWave = NewObject<UBYGTextToSpeechSoundWave>();
	TTSSoundWave->Initialize( VoiceRequiredAttributes, VoiceOptionalAttributes, Rate, Text );
	return TTSSoundWave;
}

bool UBYGTextToSpeechStatics::SpeakText( const FText& Text )
{

	// Use the defaults from the module


	return true;
}

USoundWave* UBYGTextToSpeechStatics::SpeakTextAll( const FText& Text, EBYGSpeakerGender Gender, const FString& Locale, int32 Rate )
{
#if PLATFORM_WINDOWS
	//vendor=microsoft;language=409

	const LCID Lcid = LocaleNameToLCID( *Locale, 0 );

	const FString GenderString = Gender == EBYGSpeakerGender::Masculine ? "male" : "female";
	const FString VoiceRequiredAttributes = FString::Printf( TEXT( "vendor=microsoft;language=%x" ), Lcid );
	const FString VoiceOptionalAttributes = FString::Printf( TEXT( "gender=%s" ), *GenderString );

	UBYGTextToSpeechSoundWave* TTSSoundWave = NewObject<UBYGTextToSpeechSoundWave>();
	TTSSoundWave->Initialize( VoiceRequiredAttributes, VoiceOptionalAttributes, Rate, Text.ToString() );

	return TTSSoundWave;
#else
	return nullptr;
#endif
}


void UBYGTextToSpeechStatics::GetAllVoiceInfo()
{
	if ( FAILED( ::CoInitialize( NULL ) ) )
		return;

	HRESULT hr = S_OK;

	CComPtr<ISpVoice> cpVoice; //Will send data to ISpStream
	CComPtr<ISpStream> cpStream; //Will contain IStream
	CComPtr<IStream> cpBaseStream; //raw data
	ISpObjectToken* cpToken( NULL ); //Will set voice characteristics

	//GUID guidFormat;
	WAVEFORMATEX* pWavFormatEx = nullptr;

	hr = cpVoice.CoCreateInstance( CLSID_SpVoice );

	CComPtr<ISpObjectTokenCategory> cpSpCategory = NULL;
	if ( SUCCEEDED( hr = SpGetCategoryFromId( SPCAT_VOICES, &cpSpCategory ) ) )
	{
		CComPtr<IEnumSpObjectTokens> cpSpEnumTokens;
		if ( SUCCEEDED( hr = cpSpCategory->EnumTokens( NULL, NULL, &cpSpEnumTokens ) ) )
		{
			CComPtr<ISpObjectToken> pSpTok;
			while ( cpSpEnumTokens->Next( 1, &pSpTok, NULL ) == S_OK )
			{
				CComPtr<ISpDataKey> cpSpAttributesKey;
				if ( SUCCEEDED( hr = pSpTok->OpenKey( L"Attributes", &cpSpAttributesKey ) ) )
				{
					CSpDynamicString dstrName;
					cpSpAttributesKey->GetStringValue( L"Name", &dstrName );
					CSpDynamicString dstrGender;
					cpSpAttributesKey->GetStringValue( L"Gender", &dstrGender );
					// dstrName: Microsoft David Desktop
					// dstrGender: Male
					cpSpAttributesKey.Release();
				}
				pSpTok.Release();
			}
			#if 0
			CComPtr<ISpObjectToken> pSpTok;
			while ( SUCCEEDED( hr = cpSpEnumTokens->Next( 1, &pSpTok, NULL ) ) )
			{
				// do something with the token here; for example, set the voice
				//pVoice->SetVoice( pSpTok, FALSE );
				WCHAR* pID = NULL;
				hr = pSpTok->GetId( &pID );
				// This succeeds, pID is "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Voices\Tokens\TTS_MS_EN-US_DAVID_11.0" 
				WCHAR* pName = NULL;
				pSpTok->GetStringValue( L"name", &pName );
				// pName, pGender and pLanguage are all null
				WCHAR* pGender = NULL;
				pSpTok->GetStringValue( L"gender", &pGender );
				WCHAR* pLanguage = NULL;
				pSpTok->GetStringValue( L"language", &pLanguage );
				LONG index = 0;
				WCHAR* key = NULL;
				while ( SUCCEEDED( hr = pSpTok->EnumKeys( index, &key ) ) )
				{
					// Gets some elements
					WCHAR* pValue = NULL;
					pSpTok->GetStringValue( key, &pValue );
					// Loops once, key value is "Attributes"
					index++;
				}
				index = 0;
				while ( SUCCEEDED( hr = pSpTok->EnumValues( index, &key ) ) )
				{
					// Loops many times, but none of these have what I need
					WCHAR* pValue = NULL;
					pSpTok->GetStringValue( key, &pValue );
					index++;
				}
				// NOTE:  IEnumSpObjectTokens::Next will *overwrite* the pointer; must manually release
				pSpTok.Release();
			}
			#endif
		}
	}
}
