#pragma once

#include "BYGTextToSpeechStatics.generated.h"

USTRUCT( BlueprintType )
struct BYGTEXTTOSPEECH_API FBYGVoiceInfo
{
	GENERATED_BODY()

public:
	// e.g. HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Voices\Tokens\TTS_MS_EN-US_DAVID_11.0" 
	UPROPERTY( BlueprintReadOnly )
	FString ID;

	// e.g. 
	UPROPERTY( BlueprintReadOnly )
	FString Name;

	// Masculine, Feminine or Undefined
	UPROPERTY( BlueprintReadOnly )
	EBYGSpeakerGender Gender;

	// e.g. Adult
	UPROPERTY( BlueprintReadOnly )
	FString Age;

	// e.g. Microsoft
	UPROPERTY( BlueprintReadOnly )
	FString Vendor;

	// e.g. 11.0
	UPROPERTY( BlueprintReadOnly )
	FString Version;

	// This is a Windows Language ID (LCID) in hex, e.g. 809 for English
	UPROPERTY( BlueprintReadOnly )
	FString LanguageID;
	UPROPERTY( BlueprintReadOnly )
	FString LocaleName;
};

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
	static USoundWave* SpeakTextAll( const FText& Text, EBYGSpeakerGender Gender, const FString& Locale, int32 Rate );

	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech" )
	static TArray<FBYGVoiceInfo> GetAllVoiceInfo();
};
