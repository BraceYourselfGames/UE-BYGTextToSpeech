#include "BYGTextToSpeechSoundWave.h"
#include "FMRTTSLib.h"

bool UBYGTextToSpeechSoundWave::Initialize(const FString &VoiceRequiredAttributes, const FString &VoiceOptionalAttributes, int32 Rate, const FString &Text)
{
	unsigned long BytesRead;
	int exitCode = -1;
	uint8 *TTSAudioBuffer = (uint8*)FMRTTSLib::FMRTTSLibMain::TextToWav(*VoiceRequiredAttributes, *VoiceOptionalAttributes, Rate, *Text, &BytesRead, exitCode);

	
	if (TTSAudioBuffer)
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
