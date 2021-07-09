#pragma once

#include "Sound/SoundWaveProcedural.h"
#include "BYGTextToSpeechSoundWave.generated.h"

UCLASS()
class UBYGTextToSpeechSoundWave : public USoundWaveProcedural
{
	GENERATED_BODY()
public:
	bool Initialize( const FString& VoiceRequiredAttributes, const FString& VoiceOptionalAttributes, int32 Rate, const FString& Text );
};
