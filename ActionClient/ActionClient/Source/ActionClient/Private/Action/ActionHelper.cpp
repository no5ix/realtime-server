// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionHelper.h"

bool ActionHelper::bShowDebugMessages = true;

// ScreenMsg
void ActionHelper::ScreenMsg( const FString& Msg )
{
	if (!bShowDebugMessages) return;
	GEngine->AddOnScreenDebugMessage( -1, 66.f, FColor::Red, *( STR_CUR_CLASS_LINE + "  :  " + *Msg ) );
}

void ActionHelper::ScreenMsg( const FString& Msg, const FString& Msg2 )
{
	if (!bShowDebugMessages) return;
	GEngine->AddOnScreenDebugMessage( -1, 66.f, FColor::Red, *( STR_CUR_CLASS_LINE + "  :  " + *Msg + "      " + *Msg2 ) );
}

void ActionHelper::ScreenMsg( const FString& Msg, const float FloatValue )
{
	if (!bShowDebugMessages) return;
	GEngine->AddOnScreenDebugMessage( -1, 66.f, FColor::Red, *( STR_CUR_CLASS_LINE + "  :  " + Msg + "      " + FString::SanitizeFloat( FloatValue ) ) );
}

// OutputLog
void ActionHelper::OutputLog( const FString& Msg )
{
	if (!bShowDebugMessages) return;
	UE_LOG( LogTemp, Warning, TEXT( "%s  :  %s" ), *STR_CUR_CLASS_LINE, *FString( Msg ) );
};

void ActionHelper::OutputLog( const FString& Msg, const FString& Msg2 )
{      
	if (!bShowDebugMessages) return;
	UE_LOG( LogTemp, Warning, TEXT( "%s  :  %s     %s" ), *STR_CUR_CLASS_LINE, *FString( Msg ), *FString( Msg2 ) );
}

void ActionHelper::OutputLog( const FString& Msg, const float FloatValue )
{      
	if (!bShowDebugMessages) return;
	UE_LOG( LogTemp, Warning, TEXT( "%s  :  %s    %f" ), *STR_CUR_CLASS_LINE, *FString( Msg ), FloatValue );
}

