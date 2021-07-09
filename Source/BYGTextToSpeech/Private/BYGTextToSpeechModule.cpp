// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGTextToSpeechModule.h"
#include "BYGTextToSpeechStatics.h"

#define LOCTEXT_NAMESPACE "BYGTextToSpeechModule"

void FBYGTextToSpeechModule::StartupModule()
{
	LastFrameNumberWeTicked = INDEX_NONE;

	UBYGTextToSpeechStatics::GetAllVoiceInfo();

	FDefaultGameModuleImpl::StartupModule();
}

void FBYGTextToSpeechModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
}

void GetChildrenAccessibleText( TSharedRef<SWidget> Owner, TArray<FText>& OutText )
{
	FChildren* AllChildren = Owner->GetAllChildren();
	if ( AllChildren )
	{
		for ( int32 i = 0; i < AllChildren->Num(); ++i )
		{
			TSharedRef<SWidget> Child = AllChildren->GetChildAt( i );
			const FText Text = Child->GetAccessibleText();
			if ( !Text.IsEmpty() )
			{
				OutText.Add( Text );
			}
			GetChildrenAccessibleText( Child, OutText );
		}
	}
}

void FBYGTextToSpeechModule::Tick( float DeltaTime )
{
	if ( LastFrameNumberWeTicked != GFrameCounter )
	{

		// Find the widget that we are over, and any text widgets that beneath
		// it
		const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
		const FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse( CursorPos, FSlateApplication::Get().GetInteractiveTopLevelWindows() );

		static const FString RootName = "SGameLayerManager";

		// Anything inside here, and we're hovering over one of our widgets
		if ( WidgetPath.IsValid() )
		{
			for ( int32 i = 0; i < WidgetPath.Widgets.Num(); ++i )
			{
				const FArrangedWidget& ArrangedWidget = WidgetPath.Widgets[ i ];
				const TSharedRef<SWidget> Widget = ArrangedWidget.Widget;

				if ( Widget->GetTypeAsString() == RootName )
				{
					TSharedRef<SWidget> LeafWidget = WidgetPath.GetLastWidget();
					TArray<FText> AllText;
					GetChildrenAccessibleText( LeafWidget, AllText );
					break;
				}
			}
		}

		LastFrameNumberWeTicked = GFrameCounter;
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGTextToSpeechModule, BYGTextToSpeech );


