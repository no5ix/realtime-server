// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionHelper.h"

bool ActionHelper::ShowOnScreenDebugMessages = true;

//ScreenMsg
void ActionHelper::ScreenMsg( const FString& Msg )
{
	if (!ShowOnScreenDebugMessages) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, *Msg );
}

void ActionHelper::ScreenMsg( const FString& Msg, const float Value )
{
	if (!ShowOnScreenDebugMessages) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, FString::Printf( TEXT( "%s %f" ), *Msg, Value ) );
}

void ActionHelper::ScreenMsg( const FString& Msg, const FString& Msg2 )
{
	if (!ShowOnScreenDebugMessages) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, FString::Printf( TEXT( "%s %s" ), *Msg, *Msg2 ) );
}
