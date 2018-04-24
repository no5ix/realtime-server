// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionHelper.h"


//ScreenMsg
void ActionHelper::ScreenMsg( const FString& Msg )
{
	if (!ACTION_SHOW_DEBUG_MESSAGE) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, *Msg );
}

void ActionHelper::ScreenMsg( const FString& Msg, const FString& Msg2 )
{
	if (!ACTION_SHOW_DEBUG_MESSAGE) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, FString::Printf( TEXT( "%s %s" ), *Msg, *Msg2 ) );
}

void ActionHelper::ScreenMsg( const FString& Msg, const float Value )
{
	if (!ACTION_SHOW_DEBUG_MESSAGE) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, FString::Printf( TEXT( "%s %f" ), *Msg, Value ) );
}

