#pragma once

#include "BYGTextToSpeechSettings.h"
#include "BYGTextToSpeechStatics.generated.h"

USTRUCT( BlueprintType )
struct BYGTEXTTOSPEECH_API FBYGVoiceInfo
{
	GENERATED_BODY()

public:
	// e.g. HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Voices\Tokens\TTS_MS_EN-US_DAVID_11.0" 
	UPROPERTY( BlueprintReadOnly )
	FString Id;

	// Human-readable voice name e.g. Microsoft David Desktop
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

	// This is a Windows Language ID (LCID) in hex, e.g. 409 for US English
	UPROPERTY( BlueprintReadOnly )
	FString LanguageID;

	// 2 or 5-character locale code e.g. en-US, fr-FR
	UPROPERTY( BlueprintReadOnly )
	FString LocaleName;
};

UCLASS( meta = ( ScriptName = "BYGTextToSpeechStatics" ) )
class BYGTEXTTOSPEECH_API UBYGTextToSpeechStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Use module defaults
	//UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech" )
	//static bool SpeakText( const FText& Text );

	// 
	/** 
	 * Speak the text using a 2D listener
	 * @return Whether the text was successfully spoken or not
	 */
	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech", meta = ( WorldContext = "WorldContextObject", AdvancedDisplay = 1, UnsafeDuringActorConstruction = "true" ) )
	static bool SpeakText( const UObject* WorldContextObject, const FText& Text, const FString& Locale = "en-US", EBYGSpeakerGender Gender = EBYGSpeakerGender::Undefined, int32 Speed = 5 );

	/** 
	 * Creates and returns a Sound Wave containing the spoken text
	 * @param Text - The text to be spoken
	 * @param Locale - If the locale is not found, the voice will not play
	 * @param Gender - Request that the voice is masculine, feminine or do not specify
	 * @param Speed - An integer that controls how fast the voice speaks. 5 is "normal", can accept 0~10 inclusive
	 * @return A USoundWave containing the speech data
	 */
	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech", meta = ( AdvancedDisplay = 1 ) )
	static USoundWave* TextToSoundWave( const FText& Text, const FString& Locale = "en-US", EBYGSpeakerGender Gender = EBYGSpeakerGender::Undefined, int32 Speed = 5);

	// This is a more raw version of the text to wave
	/** 
	 * This is a more raw version of the text to wave
	 * @return An array of Voice Info structs
	 */
	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech" )
	static USoundWave* TextToSoundWaveAdvanced( FString VoiceRequiredAttributes = "vendor=microsoft;language=409", FString VoiceOptionalAttributes = "", int32 Rate = 0, FString Text = "test" );

	/** 
	 * Returns a list of all the voices that are installed on the machine
	 * @return An array of Voice Info structs
	 */
	UFUNCTION( BlueprintCallable, Category = "BYG Text To Speech" )
	static TArray<FBYGVoiceInfo> GetVoices();
};
