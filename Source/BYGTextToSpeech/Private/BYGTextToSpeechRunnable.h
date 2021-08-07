// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

// Have to use typedef because of macro
//DECLARE_DELEGATE_OneParam( FBYGOnTTSCompleteSignature, class USoundWave* /*SoundWave*/ );

struct FBYGSoundWaveData
{
	unsigned long BytesRead = 0;
	//TUniquePtr<uint8> Buffer;
	uint8* Buffer = nullptr;

	// Better pray it's been used by now
	virtual ~FBYGSoundWaveData()
	{
		//delete Buffer;
	}
};

class FBYGTextToSpeechRunnable : public FRunnable
{
public:
	FBYGTextToSpeechRunnable();
	virtual ~FBYGTextToSpeechRunnable();

	// FRunnable functions
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// FRunnable

	void SetRate( int32 Rate );
	void SetAttributes( const FString& Attributes );

	void AddText( const FString& Text );
	void AddText( const TArray<FString>& Text );
	void ClearQueue();

	bool IsRunning() const
	{
		return bRunning;
	}

	int32 GetTextQueueSize() const;
	int32 GetSoundWaveDataSize() const;

	TArray<FBYGSoundWaveData> GetAndClearSoundWaves();

protected:
	HRESULT WaitAndPumpMessagesWithTimeout( void* hWaitHandle, DWORD dwMilliseconds );
	char* TextToWavInner( const wchar_t* voiceRequiredAttributes, const wchar_t* voiceOptionalAttributes, long rate, const wchar_t* textToRender, ULONG* pBytesRead );

	TArray<FString> TextQueue;
	TArray<FBYGSoundWaveData> SoundWaveData;

	bool bRunning = false;

	FString Attributes = "";
	int32 Rate = 0;

	bool bStopInner = false;

	class FRunnableThread* Thread = nullptr;

	mutable FCriticalSection TextCriticalSection;
	mutable FCriticalSection SoundCriticalSection;
};
