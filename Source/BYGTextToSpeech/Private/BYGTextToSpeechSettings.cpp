// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechSettings.h"

UBYGTextToSpeechSettings::UBYGTextToSpeechSettings( const FObjectInitializer& ObjectInitializer )
{
}


bool UBYGTextToSpeechSettings::Validate()
{
	bool bAnyChanges = false;

	// look for dups?

	return bAnyChanges;
}

#if WITH_EDITOR
void UBYGTextToSpeechSettings::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent )
{
#if 0
	if (
		( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGTextToSpeechSettings, PrimaryLanguageCode ) )
		)
	{
	}
	#endif

	Super::PostEditChangeProperty( PropertyChangedEvent );

}
#endif
