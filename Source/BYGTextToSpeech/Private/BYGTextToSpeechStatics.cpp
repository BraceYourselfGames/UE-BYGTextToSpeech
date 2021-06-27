#include "BYGTextToSpeechStatics.h"
#include "BYGTextToSpeechSoundWave.h"
#include <windows.h>
#include <winnls.h>

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

bool UBYGTextToSpeechStatics::SpeakTextAll( const FText& Text, EBYGSpeakerGender Gender, const FString& Locale, int32 Rate )
{
	//vendor=microsoft;language=409

	const LCID Lcid = LocaleNameToLCID( *Locale, 0 );

	const FString GenderString = Gender == EBYGSpeakerGender::Masculine ? "Male" : "Female";
	const FString VoiceRequiredAttributes = FString::Printf( TEXT( "vendor=microsoft;language=%d;gender=%s" ), Lcid, *GenderString);
	const FString VoiceOptionalAttributes = "";

	auto TTSSoundWave = NewObject<UBYGTextToSpeechSoundWave>();
	TTSSoundWave->Initialize( VoiceRequiredAttributes, VoiceOptionalAttributes, Rate, Text.ToString() );

	return true;
}
