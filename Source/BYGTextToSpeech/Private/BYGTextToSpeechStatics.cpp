#include "BYGTextToSpeechStatics.h"
#include "BYGTextToSpeechSoundWave.h"
#if PLATFORM_WINDOWS

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

#include "Kismet/GameplayStatics.h"

bool UBYGTextToSpeechStatics::SpeakText( const UObject* WorldContextObject, const FText& Text, const FString& Locale, EBYGSpeakerGender Gender, int32 Speed )
{
	USoundWave* SoundWave = TextToSoundWave( Text, Locale, Gender, Speed );
	if ( SoundWave )
	{
		UGameplayStatics::PlaySound2D( WorldContextObject, SoundWave );
		return true;
	}
	return false;
}


USoundWave* UBYGTextToSpeechStatics::TextToSoundWave( const FText& Text, const FString& Locale, EBYGSpeakerGender Gender, int32 Speed )
{
#if PLATFORM_WINDOWS
	//vendor=microsoft;language=409


	TArray<FString> RequiredAttributes;
	// Convert from en-US to integer
	const LCID Lcid = LocaleNameToLCID( *Locale, 0 );
	// Print as hex
	RequiredAttributes.Add( FString::Printf( TEXT( "language=%x" ), Lcid ) );
	//RequiredAttributes.Add( FString::Printf( TEXT( "vendor=%s" ), *Vendor ) );

	const FString VoiceRequiredAttributes = FString::Join( RequiredAttributes, TEXT( ";" ) );

	TArray<FString> OptionalAttributes;
	if ( Gender != EBYGSpeakerGender::Undefined )
	{
		const FString GenderString = Gender == EBYGSpeakerGender::Masculine ? "male" : "female";
		OptionalAttributes.Add( FString::Printf( TEXT( "gender=%s" ), *GenderString ) );
	}
	const FString VoiceOptionalAttributes = FString::Join( OptionalAttributes, TEXT( ";" ) );

	const int32 Rate = FMath::GetMappedRangeValueClamped( FVector2D( 0, 10 ), FVector2D( -10, 10 ), Speed );

	UBYGTextToSpeechSoundWave* TTSSoundWave = NewObject<UBYGTextToSpeechSoundWave>();
	TTSSoundWave->Initialize( VoiceRequiredAttributes, VoiceOptionalAttributes, Rate, Text.ToString() );

	return TTSSoundWave;
#else
	return nullptr;
#endif
}

USoundWave* UBYGTextToSpeechStatics::TextToSoundWaveAdvanced( FString VoiceRequiredAttributes, FString VoiceOptionalAttributes, int32 Rate, FString Text )
{
	UBYGTextToSpeechSoundWave* TTSSoundWave = NewObject<UBYGTextToSpeechSoundWave>();
	TTSSoundWave->Initialize( VoiceRequiredAttributes, VoiceOptionalAttributes, Rate, Text );
	return TTSSoundWave;
}


TArray<FBYGVoiceInfo> UBYGTextToSpeechStatics::GetVoices()
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
