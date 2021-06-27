#pragma once

#include "BYGTextToSpeechStatics.generated.h"

UCLASS()
class BYGTEXTTOSPEECH_API UBYGTextToSpeechStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech" )
	static USoundWave* TextToWave( FString VoiceRequiredAttributes = "vendor=microsoft;language=409", FString VoiceOptionalAttributes = "", int32 Rate = 0, FString Text = "test" );

	// Use module defaults
	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech" )
	static bool SpeakText( const FText& Text );

	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech" )
	static bool SpeakTextAll( const FText& Text, EBYGSpeakerGender Gender, const FString& Locale, int32 Rate );
};
