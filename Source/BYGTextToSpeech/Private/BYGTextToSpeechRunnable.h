// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

// Have to use typedef because of macro
//DECLARE_DELEGATE_OneParam( FBYGOnTTSCompleteSignature, class USoundWave* /*SoundWave*/ );

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
	//FBYGOnTTSCompleteSignature OnTTSCompleteDelegate;

	TArray<class USoundWave*> GetAndClearSoundWaves();

protected:
	TArray<FString> TextQueue;
	TArray<class USoundWave*> SoundWaves;

	bool bRunning = false;

	bool bStopThread = false;
	bool bIsComplete = false;

	FString Attributes = "";
	int32 Rate = 0;

	class FRunnableThread* Thread = nullptr;

	mutable FCriticalSection TextCriticalSection;
};
