// Fill out your copyright notice in the Description page of Project Settings.


#include "RealTimeSrvHelper.h"
//#include "RealTimeSrvTiming.h"


//ScreenMsg
void RealTimeSrvHelper::ScreenMsg( const FString& Msg )
{
	if (!RTS_SHOW_DEBUG_SCREEN_MSG) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, *Msg );
}

void RealTimeSrvHelper::ScreenMsg( const FString& Msg, const FString& Msg2 )
{
	if (!RTS_SHOW_DEBUG_SCREEN_MSG) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, FString::Printf( TEXT( "%s %s" ), *Msg, *Msg2 ) );
}

void RealTimeSrvHelper::ScreenMsg( const FString& Msg, const float Value )
{
	if (!RTS_SHOW_DEBUG_SCREEN_MSG) return;
	GEngine->AddOnScreenDebugMessage( -1, 55.f, FColor::Red, FString::Printf( TEXT( "%s %f" ), *Msg, Value ) );
}

bool RealTimeSrvHelper::SequenceGreaterThanOrEqual( PacketSN s1, PacketSN s2 )
{
	return ( ( s1 >= s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
}


bool RealTimeSrvHelper::SequenceGreaterThan( PacketSN s1, PacketSN s2 )
{
	return ( ( s1 > s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
}

bool RealTimeSrvHelper::ChunkPacketIDGreaterThanOrEqual( ChunkPacketID s1, ChunkPacketID s2 )
{
	return ( ( s1 >= s2 ) && ( s1 - s2 <= HALF_MAX_CHUNK_PACKET_ID ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_CHUNK_PACKET_ID ) );
}


bool RealTimeSrvHelper::ChunkPacketIDGreaterThan( ChunkPacketID s1, ChunkPacketID s2 )
{
	return ( ( s1 > s2 ) && ( s1 - s2 <= HALF_MAX_CHUNK_PACKET_ID ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_CHUNK_PACKET_ID ) );
}

