// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject/Public/UObject/NoExportTypes.h"
#include "Engine/EngineTypes.h"
#include "BYGTextToSpeechSettings.generated.h"

UENUM()
enum class EBYGSpeakerGender : uint8
{
	Masculine,
	Feminine
};

UCLASS( config = BYGTextToSpeech, defaultconfig )
class BYGTEXTTOSPEECH_API UBYGTextToSpeechSettings : public UObject
{
	GENERATED_BODY()

public:
	UBYGTextToSpeechSettings( const FObjectInitializer& ObjectInitializer );

	UPROPERTY( config, EditAnywhere, Category = "Text to Speech" )
	FString DefaultLocale = "en";

	UPROPERTY( config, EditAnywhere, Category = "Text to Speech" )
	EBYGSpeakerGender DefaultGender = EBYGSpeakerGender::Masculine;

	UPROPERTY( config, EditAnywhere, Category = "Text to Speech", meta = ( UIMin = 0, UIMax = 10 ) )
	int32 DefaultRate = 5;

	bool Validate();

#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;
#endif
};
