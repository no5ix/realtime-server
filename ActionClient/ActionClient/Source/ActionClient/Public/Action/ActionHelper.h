// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ActionClient.h"


// 是否要显示调试打印信息
#define ACTION_SHOW_DEBUG_SCREEN_MSG					false
#define ACTION_SHOW_DEBUG_OUTPUT_LOG					false


//Current Class Name + Function Name where this is called!
#define STR_CUR_CLASS_FUNC							(FString(__FUNCTION__))

//Current Class where this is called!
#define STR_CUR_CLASS								(FString(__FUNCTION__).Left(FString(__FUNCTION__).Find(TEXT(":"))) )

//Current Function Name where this is called!
#define STR_CUR_FUNC								(FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2 ))

//Current Line Number in the code where this is called!
#define STR_CUR_LINE								(FString::FromInt(__LINE__))

//Current Class and Line Number where this is called!
#define STR_CUR_CLASS_LINE							(STR_CUR_CLASS + "(" + STR_CUR_LINE + ")")

//Current Class and Line Number where this is called!
#define STR_CUR_CLASS_FUNC_LINE						(STR_CUR_CLASS_FUNC + "(" + STR_CUR_LINE + ")")

//Current Function Signature where this is called!
#define STR_CUR_FUNCSIG								(FString(__FUNCSIG__))


////////////// Victory Screen Message
// 	Gives you the Class name and exact line number where you print a message to yourself!


#define A_SCREENMSG()								if (ACTION_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, 66.f, FColor::Red, *(STR_CUR_CLASS_FUNC_LINE )) )

#define A_SCREENMSG_1(StringParam1)                        if (ACTION_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, 66.f, FColor::Red, *(STR_CUR_CLASS_FUNC_LINE + "  :  " + StringParam1)) )

#define A_SCREENMSG_2(StringParam1, StringParam2)     			if (ACTION_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, 66.f, FColor::Red, *(STR_CUR_CLASS_FUNC_LINE + "  :  " + StringParam1 + "      " + StringParam2)) )

//#define A_SCREENMSG_F(StringParam1, NumericalParam2)     		if (ACTION_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, 66.f, FColor::Red, *(STR_CUR_CLASS_FUNC_LINE + "  :  " + StringParam1 + "      " + FString::SanitizeFloat(NumericalParam2))) )
#define A_SCREENMSG_N(StringParam1, NumericalParam2)     		if (ACTION_SHOW_DEBUG_SCREEN_MSG) (GEngine->AddOnScreenDebugMessage(-1, 66.f, FColor::Red, FString::Printf( TEXT("%s  :  %s    %f"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), float(NumericalParam2) ) ) )



///////// UE LOG!

// Example usage: A_LOG( "Action!" );
#define	A_LOG() 		           					if (ACTION_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s"), *STR_CUR_CLASS_FUNC_LINE )

// Example usage: A_LOG( "Action!" );
#define A_LOG_1(StringParam1) 		           				if (ACTION_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1))

// Example usage: A_LOG2("Action!", "Cut!");
#define A_LOG_2(StringParam1, StringParam2) 	       				if (ACTION_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s     %s"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), *FString(StringParam2))

// Example usage: A_LOGF("Action!", 88.f);
#define A_LOG_N(StringParam1, NumericalParam2) 	       		if (ACTION_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s    %f"), *STR_CUR_CLASS_FUNC_LINE, *FString(StringParam1), float(NumericalParam2) )

/*
A_LOGM is special in that you can have an arbitrary number of vars that you output, and of any type, similar to standard UE_LOG functionality!
Example usage :

int32 Health = 100;
float ArmorPct = 52.33;
FVector Location(33,12,1);
A_LOGM("Health: %d, ArmorPct: %f, Loc: %s",  Health, ArmorPct, *Location.ToString());
*/
#define A_LOG_M(FormatString, ...)     				if (ACTION_SHOW_DEBUG_OUTPUT_LOG) UE_LOG(LogTemp, Warning, TEXT("%s  :  %s"), *STR_CUR_CLASS_FUNC_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) )



class ActionHelper
{
public:

	static void ScreenMsg( const FString& Msg );
	static void ScreenMsg( const FString& Msg, const FString& Msg2 );
	static void ScreenMsg( const FString& Msg, const float FloatValue );
};