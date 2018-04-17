// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ActionClient.h"
/**
 * 
 */
class ActionHelper
{
public:
	static bool ShowOnScreenDebugMessages;

	static void ScreenMsg( const FString& Msg );

	static void ScreenMsg( const FString& Msg, const float Value );

	static void ScreenMsg( const FString& Msg, const FString& Msg2 );
};
