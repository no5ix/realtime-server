// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "ActionSocketUtil.h"
//#include "Action.h"

ActionSocketUtil::ActionSocketUtil()
{
}

ActionSocketUtil::~ActionSocketUtil()
{
}

void ActionSocketUtil::CreateUDPSocket(FSocket*& inSocket, const FString& inYourChosenSocketName)
{
	inSocket = FUdpSocketBuilder( *inYourChosenSocketName )
		.AsReusable()
		.WithBroadcast()
		;

	//check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

	//Set Send Buffer Size
	int32 SendSize = 2 * 1024 * 1024;
	inSocket->SetSendBufferSize( SendSize, SendSize );
	inSocket->SetReceiveBufferSize( SendSize, SendSize );
}

bool ActionSocketUtil::CreateInternetAddress(TSharedPtr<FInternetAddr>& inRemoteAddr, const FString& inIP, const int32 inPort )
{
	//Create Remote Address.
	inRemoteAddr = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->CreateInternetAddr();

	bool bIsValid;
	inRemoteAddr->SetIp( *inIP, bIsValid );
	inRemoteAddr->SetPort( inPort );

	if (!bIsValid)
	{
		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( " IP address was not valid!!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
	}
	return bIsValid;
}

bool ActionSocketUtil::SendTo( FSocket*& inSocket, const OutputMemoryBitStream& inOutputStream, int32& inByteSent, const TSharedPtr<FInternetAddr>& inRemoteAddr )
{
	if (!inSocket)
	{
		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( " No SendTo socket!!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
		return false;
	}

	inSocket->SendTo( ( uint8* )inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength(), inByteSent, *inRemoteAddr );//发送给远端地址  
	
	return true;
}

bool ActionSocketUtil::RecvFrom( FSocket*& inSocket, char * inData, int32 inBufferSize, int32& inRefReadByteCount, TSharedRef<FInternetAddr>& inFromAddress)
{
	if (!inSocket)
	{
		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( " No RecvFrom socket!!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
		return false;
	}

	inSocket->RecvFrom( ( uint8* )inData, inBufferSize, inRefReadByteCount, *inFromAddress );//创建远程接收地址  

	return true;

	
}
