// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ActionClient.h"



//Current Class Name + Function Name where this is called!
#define STR_CUR_CLASS_FUNC (FString(__FUNCTION__))

//Current Class where this is called!
#define STR_CUR_CLASS (FString(__FUNCTION__).Left(FString(__FUNCTION__).Find(TEXT(":"))) )

//Current Function Name where this is called!
#define STR_CUR_FUNC (FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2 ))

//Current Line Number in the code where this is called!
#define STR_CUR_LINE  (FString::FromInt(__LINE__))

//Current Class and Line Number where this is called!
#define STR_CUR_CLASS_LINE (STR_CUR_CLASS + "(" + STR_CUR_LINE + ")")

//Current Function Signature where this is called!
#define STR_CUR_FUNCSIG (FString(__FUNCSIG__))

/*
V_LOGM is special in that you can have an arbitrary number of vars that you output, and of any type, similar to standard UE_LOG functionality!
Example usage :
int32 Health = 100;
float ArmorPct = 52.33;
FVector Location( 33, 12, 1 );
V_LOGM( "Health: %d, ArmorPct: %f, Loc: %s", Health, ArmorPct, *Location.ToString() );
*/
#define A_LOG(FormatString, ...)     UE_LOG(LogTemp, Warning, TEXT("%s  :  %s"), *STR_CUR_CLASS_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) )


class ActionHelper
{
public:
	static bool bShowDebugMessages;

	static void ScreenMsg( const FString& Msg );
	static void ScreenMsg( const FString& Msg, const FString& Msg2 );
	static void ScreenMsg( const FString& Msg, const float FloatValue );

	static void OutputLog( const FString& Msg );
	static void OutputLog( const FString& Msg, const FString& Msg2 );
	static void OutputLog( const FString& Msg, const float FloatValue );
};
