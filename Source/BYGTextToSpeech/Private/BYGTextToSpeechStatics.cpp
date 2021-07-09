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

#include <atlbase.h>
#include <sapi.h>
#include <sphelper.h>

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

TArray<FBYGVoiceInfo> UBYGTextToSpeechStatics::GetAllVoiceInfo()
{
	TArray<FBYGVoiceInfo> Voices;

	if ( FAILED( ::CoInitialize( NULL ) ) )
		return Voices;

	HRESULT hr = S_OK;

	CComPtr<ISpVoice> cpVoice; //Will send data to ISpStream
	CComPtr<ISpStream> cpStream; //Will contain IStream
	CComPtr<IStream> cpBaseStream; //raw data
	ISpObjectToken* cpToken( NULL ); //Will set voice characteristics

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
				FBYGVoiceInfo VoiceInfo;

				WCHAR* pID = NULL;
				hr = pSpTok->GetId( &pID );
				if ( SUCCEEDED( hr ) )
				{
					VoiceInfo.ID = FString( pID );
				}

				// Available keys: Name, Gender, Age, Language, Vendor, Version
				CComPtr<ISpDataKey> cpSpAttributesKey;
				if ( SUCCEEDED( hr = pSpTok->OpenKey( L"Attributes", &cpSpAttributesKey ) ) )
				{
					WCHAR* key2 = NULL;
					LONG index2 = 0;
					while ( SUCCEEDED( hr = cpSpAttributesKey->EnumValues( index2, &key2 ) ) )
					{
						WCHAR* pValue = NULL;
						cpSpAttributesKey->GetStringValue( key2, &pValue );
						index2++;
					}

					CSpDynamicString dstrName;
					cpSpAttributesKey->GetStringValue( L"Name", &dstrName );
					VoiceInfo.Name = FString( ( WCHAR* )dstrName );

					CSpDynamicString dstrGender;
					cpSpAttributesKey->GetStringValue( L"Gender", &dstrGender );
					FString Gender( ( WCHAR* )dstrGender );
					if ( Gender == "Male" )
					{
						VoiceInfo.Gender = EBYGSpeakerGender::Masculine;
					}
					else if ( Gender == "Female" )
					{
						VoiceInfo.Gender = EBYGSpeakerGender::Feminine;
					}
					else
					{
						VoiceInfo.Gender = EBYGSpeakerGender::Undefined;
					}

					CSpDynamicString dstrAge;
					cpSpAttributesKey->GetStringValue( L"Age", &dstrAge );
					VoiceInfo.Age = FString( ( WCHAR* )dstrAge );

					CSpDynamicString dstrVendor;
					cpSpAttributesKey->GetStringValue( L"Vendor", &dstrVendor );
					VoiceInfo.Vendor = FString( ( WCHAR* )dstrVendor );

					CSpDynamicString dstrVersion;
					cpSpAttributesKey->GetStringValue( L"Version", &dstrVersion );
					VoiceInfo.Version = FString( ( WCHAR* )dstrVersion );

					CSpDynamicString dstrLanguage;
					cpSpAttributesKey->GetStringValue( L"Language", &dstrLanguage );
					VoiceInfo.LanguageID = FString( ( WCHAR* )dstrLanguage );

					// LCID is hex, but LCIDToLocaleName wants dec, because of course
					const char* HexCode = TCHAR_TO_ANSI( dstrLanguage );
					wchar_t wc = strtol( HexCode, NULL, 16 );
					CSpDynamicString dstrLocaleName( 16 );
					int result = LCIDToLocaleName( wc, dstrLocaleName, 16, 0 );
					VoiceInfo.LocaleName = FString( ( WCHAR* )dstrLocaleName );

					Voices.Add( VoiceInfo );

					cpSpAttributesKey.Release();
				}
				pSpTok.Release();
			}
		}
	}

	return Voices;
}
