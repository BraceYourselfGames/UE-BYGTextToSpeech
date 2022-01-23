// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechSettings.h"

UBYGTextToSpeechSettings::UBYGTextToSpeechSettings( const FObjectInitializer& ObjectInitializer )
{
	TextSplitDelimiters = {
		TEXT( "." ),
		TEXT( "\r\n" ),
		TEXT( "\n" )
	};
}
