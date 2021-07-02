#include "BYGTextToSpeechStatics.h"
#include "BYGTextToSpeechSoundWave.h"
#if PLATFORM_WINDOWS
#include "Windows/MinWindows.h"
#include <winnls.h>
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
