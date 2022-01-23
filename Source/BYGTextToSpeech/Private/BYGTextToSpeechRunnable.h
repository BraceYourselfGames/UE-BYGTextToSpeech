// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

// Have to use typedef because of macro
//DECLARE_DELEGATE_OneParam( FBYGOnTTSCompleteSignature, class USoundWave* /*SoundWave*/ );

struct FBYGSoundWaveData
{
#if 0
	FBYGSoundWaveData(
		TArray<uint8> InBuffer,
		unsigned long InBytesRead
		)
		: Buffer( MoveTemp( InBuffer ) )
		, BytesRead( InBytesRead )
	{

	}
#endif

	TArray<uint8> Buffer;
	unsigned long BytesRead = 0;
	//uint8* Buffer = nullptr;

	// Better pray it's been used by now
	//virtual ~FBYGSoundWaveData();
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

	// Transfers ownership because we only want one clear owner of the sound wave data
	void GetAndClearSoundWaves( TArray<TUniquePtr<FBYGSoundWaveData>>& OutData );

protected:
	HRESULT WaitAndPumpMessagesWithTimeout( void* hWaitHandle, DWORD dwMilliseconds );
	bool TextToWavInner( const wchar_t* voiceRequiredAttributes, const wchar_t* voiceOptionalAttributes, long rate, const wchar_t* textToRender, TArray<uint8>& OutBuffer );

	TArray<FString> TextQueue;
	TArray<TUniquePtr<FBYGSoundWaveData>> SoundWaveData;

	bool bRunning = false;

	FString Attributes = "";
	int32 Rate = 0;

	bool bCancelInnerLoop = false;

	class FRunnableThread* Thread = nullptr;

	mutable FCriticalSection TextCriticalSection;
	mutable FCriticalSection SoundCriticalSection;
};
