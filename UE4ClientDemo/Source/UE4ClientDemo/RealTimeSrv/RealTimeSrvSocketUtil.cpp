#include "RealTimeSrvSocketUtil.h"

void RealTimeSrvSocketUtil::CreateUDPSocket( FSocket*& inSocket, const FString& inYourChosenSocketName )
{
	inSocket = FUdpSocketBuilder( *inYourChosenSocketName )
		.AsReusable()
		.WithBroadcast()
	;




	int32 SendSize = 2 * 1024 * 1024;
	inSocket->SetSendBufferSize( SendSize, SendSize );
	inSocket->SetReceiveBufferSize( SendSize, SendSize );
}

bool RealTimeSrvSocketUtil::CreateInternetAddress( TSharedPtr<FInternetAddr>& inRemoteAddr, const FString& inIP, const int32 inPort )
{

	inRemoteAddr = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->CreateInternetAddr();

	bool bIsValid;
	inRemoteAddr->SetIp( *inIP, bIsValid );
	inRemoteAddr->SetPort( inPort );

	if ( !bIsValid )
	{
		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( " IP address was not valid!!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
	}
	return bIsValid;
}

bool RealTimeSrvSocketUtil::SendTo( FSocket*& inSocket, const OutputBitStream& inOutputStream, int32& inByteSent, const TSharedPtr<FInternetAddr>& inRemoteAddr )
{
	if ( !inSocket )
	{
		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( " No SendTo socket!!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
		return false;
	}

	inSocket->SendTo( ( uint8* )inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength(), inByteSent, *inRemoteAddr );

	return true;
}

bool RealTimeSrvSocketUtil::RecvFrom( FSocket*& inSocket, char * inData, int32 inBufferSize, int32& inRefReadByteCount, TSharedRef<FInternetAddr>& inFromAddress )
{
	if ( !inSocket )
	{
		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( " No RecvFrom socket!!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
		return false;
	}

	inSocket->RecvFrom( ( uint8* )inData, inBufferSize, inRefReadByteCount, *inFromAddress );

	return true;


}
