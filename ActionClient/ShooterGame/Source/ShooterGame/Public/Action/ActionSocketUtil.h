// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Networking.h"

/**
 * 
 */
class ActionSocketUtil
{
public:
	ActionSocketUtil();
	~ActionSocketUtil();

	static void CreateUDPSocket( FSocket*& inSocket, const FString& inYourChosenSocketName );

	static bool CreateInternetAddress( TSharedPtr<FInternetAddr>& inRemoteAddr, const FString& inIP, const int32 inPort );

	static bool SendTo( FSocket*& inSocket, const OutputMemoryBitStream& inOutputStream, int32& inByteSent, const TSharedPtr<FInternetAddr>& inRemoteAddr );

	static bool RecvFrom( FSocket*& inSocket, char * inData, int32 inBufferSize, int32& inRefReadByteCount, TSharedRef<FInternetAddr>& inFromAddress );

};
