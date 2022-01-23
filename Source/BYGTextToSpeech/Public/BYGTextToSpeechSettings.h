// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject/Public/UObject/NoExportTypes.h"
#include "Engine/EngineTypes.h"
#include "BYGTextToSpeechSettings.generated.h"

UENUM( BlueprintType )
enum class EBYGSpeakerGender : uint8
{
	Undefined,
	Masculine,
	Feminine
};

UCLASS( config = BYGTextToSpeech, defaultconfig )
class BYGTEXTTOSPEECH_API UBYGTextToSpeechSettings : public UObject
{
	GENERATED_BODY()

public:
	UBYGTextToSpeechSettings( const FObjectInitializer& ObjectInitializer );

	// Whether the subsystem is enabled by default
	UPROPERTY( config, EditAnywhere, Category = "Text to Speech" )
	bool bEnabled = true;
	
	// Whether the subsystem should automatically read text that it is hovered over
	UPROPERTY( config, EditAnywhere, Category = "Text to Speech", meta=(EditCondition="bEnabled") )
	bool bAutoReadOnHover = false;

	UPROPERTY( config, EditAnywhere, Category = "Text to Speech", meta=(EditCondition="bEnabled") )
	FString DefaultLocale = "en";

	// Volume multiplier. 1 is the max.
	UPROPERTY( config, EditAnywhere, Category = "Text to Speech", meta = ( UIMin = 0, UIMax = 1, EditCondition="bEnabled" ) )
	float DefaultVolumeMultiplier = 1.0f;

	//UPROPERTY( config, EditAnywhere, Category = "Text to Speech", meta=(EditCondition="bEnabled") )
	//EBYGSpeakerGender DefaultGender = EBYGSpeakerGender::Masculine;

	UPROPERTY( config, EditAnywhere, Category = "Text to Speech", meta = ( UIMin = 0, UIMax = 10, EditCondition="bEnabled" ) )
	int32 DefaultRate = 5;

	// Advanced usage. Text is split into asynchronously-parsed chunks using these characters.
	UPROPERTY( config, EditAnywhere, Category = "Text to Speech", AdvancedDisplay, meta=(EditCondition="bEnabled") )
	TArray<FString> TextSplitDelimiters;
	
	UPROPERTY( config, EditAnywhere, Category = "Text to Speech", meta=(EditCondition="bEnabled") )
	bool bShowDebugLogs = false;
};
