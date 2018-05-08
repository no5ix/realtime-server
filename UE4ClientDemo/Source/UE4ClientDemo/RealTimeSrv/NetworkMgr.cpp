// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkMgr.h"
#include "RealTimeSrvTiming.h"
#include "RealTimeSrvSocketUtil.h"
#include "InputMgr.h"
#include "RealTimeSrvHelper.h"
#include "ActionList.h"
#include "RealTimeSrvWorld.h"


std::unique_ptr<NetworkMgr> NetworkMgr::sInstance;

float NetworkMgr::kTimeBufferStatePackets = 0.132f;
float NetworkMgr::kTimeBetweenStatePackets = 0.033f;

namespace
{
	const float kTimeBetweenHellos = 1.f;
	const float kTimeBetweenInputPackets = 0.033f;
	const float kClientDisconnectTimeout = 3.f;
}

NetworkMgr::NetworkMgr() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mPlayerId( -1.f ),
	mResetedPlayerId( -1.f ),
	mDeliveryNotificationManager( true, false ),
	mState( NCS_Uninitialized ),
	mLastCheckDCTime( 0.f ),
	mLastPacketFromSrvTime( 0.f )
{
	mSocket = NULL;
}

void NetworkMgr::StaticInit( const FString& inIP, int inPort, const FString& inPlayerName )
{
	sInstance.reset( new NetworkMgr() );
	return sInstance->Init("ActionUDPSocket", inIP, inPort, inPlayerName );
}

void NetworkMgr::Init( const FString& inYourChosenSocketName, const FString& inIP, const int32 inPort, const FString& inPlayerName )
{

	mPlayerName = inPlayerName;

	RealTimeSrvSocketUtil::CreateUDPSocket( mSocket, inYourChosenSocketName );

	if ( RealTimeSrvSocketUtil::CreateInternetAddress( mRemoteAddr, inIP, inPort ) )
	{
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "ActionUDPSocket Initialized Successfully!!!" );
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}

	mState = NCS_SayingHello;

	mAvgRoundTripTime = WeightedTimedMovingAverage( 1.f );
}


void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream )
{
	int32 BytesSent = 0;
	RealTimeSrvSocketUtil::SendTo( mSocket, inOutputStream, BytesSent, mRemoteAddr );

	if (BytesSent <= 0)
	{
		const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *Str );
		RealTimeSrvHelper::ScreenMsg( Str );
	}
}

void NetworkMgr::UpdateLastPacketFromSrvTime()
{
	mLastPacketFromSrvTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
}

void NetworkMgr::SendHelloPacket()
{
	OutputBitStream helloPacket;

	helloPacket.Write( kHelloCC );
	helloPacket.Write( std::string( TCHAR_TO_UTF8( *mPlayerName ) ) );

	SendPacket( helloPacket );
	
	
	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	A_LOG_1( "****UDP**** Send Hello Packet Successfully!!!" );
	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	
}


void NetworkMgr::ProcessIncomingPackets()
{
	ReadIncomingPacketsIntoQueue();

	ProcessQueuedPackets();

	UpdateBytesSentLastFrame();
}

void NetworkMgr::ReadIncomingPacketsIntoQueue()
{

	char ReceivedDataPacketMem[1500];
	int32 packetSize = sizeof( ReceivedDataPacketMem );
	InputBitStream inputStream( ReceivedDataPacketMem, packetSize * 8 );
	int32 refReadByteCount = 0;

	TSharedRef<FInternetAddr> fromAddress = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->CreateInternetAddr();

	int receivedPackedCount = 0;

	while (receivedPackedCount < kMaxPacketsPerFrameCount)
	{
		RealTimeSrvSocketUtil::RecvFrom( mSocket, ReceivedDataPacketMem, packetSize, refReadByteCount, fromAddress );

		if (refReadByteCount == 0)
		{
			break;
		}
		else if (refReadByteCount > 0)
		{
			inputStream.ResetToCapacity( refReadByteCount );
			++receivedPackedCount;

			float simulatedReceivedTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime() + mSimulatedLatency;
			mPacketQueue.emplace( simulatedReceivedTime, inputStream, fromAddress );
		}
		else
		{
		}
	}
	
}

void NetworkMgr::ProcessQueuedPackets()
{
	while (!mPacketQueue.empty())
	{
		ReceivedPacket& nextPacket = mPacketQueue.front();
		if (RealTimeSrvTiming::sInstance.GetCurrentGameTime() > nextPacket.GetReceivedTime())
		{
			ProcessPacket( nextPacket.GetPacketBuffer());
			mPacketQueue.pop();
		}
		else
		{
			break;
		}

	}

}

void NetworkMgr::UpdateBytesSentLastFrame()
{

}


void NetworkMgr::ProcessPacket( InputBitStream& inInputStream )
{
	uint32_t	packetType;
	inInputStream.Read( packetType );

	UpdateLastPacketFromSrvTime();

	switch (packetType)
	{
	case kResetCC:
		HandleResetPacket();
		break;
	case kWelcomeCC:
		HandleWelcomePacket( inInputStream );
		break;
	case kStateCC:
 		if (mDeliveryNotificationManager.ReadAndProcessState( inInputStream ))
 		{
			HandleStatePacket( inInputStream );
 		}
 		break;
	}
}


void NetworkMgr::SendOutgoingPackets()
{
	switch (mState)
	{
	case NCS_Resetting:
	case NCS_SayingHello:
		UpdateSayingHello();
		break;
	case NCS_Reseted:
	case NCS_Welcomed:
		UpdateSendingInputPacket();
		break;
	}
}

void NetworkMgr::UpdateSayingHello()
{
	float currentTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	if (currentTime > mTimeOfLastHello + kTimeBetweenHellos)
	{
		SendHelloPacket();
		mTimeOfLastHello = currentTime;
	}
}

void NetworkMgr::CheckForDisconnects()
{

	float curTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	if ( curTime - mLastCheckDCTime < kClientDisconnectTimeout )
	{
		return;
	}
	mLastCheckDCTime = curTime;

	if ( mLastPacketFromSrvTime < curTime - kClientDisconnectTimeout )
	{
		GEngine->AddOnScreenDebugMessage( -1, 2.f, FColor::Red,
			FString::Printf( TEXT( "%s" ),
				*FString( "Connecting ..." ) )
		);
	}
}

void NetworkMgr::ResetForNewGame()
{
	float curTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	mLastCheckDCTime = curTime;
	mLastPacketFromSrvTime = curTime;
	mNetworkIdToGameObjectMap.clear();

	RealTimeSrvWorld::sInstance->ResetRealTimeSrvWorld();
	InputMgr::sInstance->GetActionList().Clear();
}

void NetworkMgr::HandleResetPacket()
{
	if (mState == NCS_Welcomed)
	{
		mState = NCS_Resetting;

		GEngine->AddOnScreenDebugMessage( -1, 6.f, FColor::Red,
			FString::Printf( TEXT( "%s" ),
			*FString( "Resetting ..." ))
		);
	}
}

void NetworkMgr::HandleWelcomePacket( InputBitStream& inInputStream )
{
	if (mState == NCS_SayingHello || mState == NCS_Resetting)
	{
		if ( mState == NCS_Resetting )
		{
			mState = NCS_Reseted;
			inInputStream.Read( mResetedPlayerId );
		}
		else
		{
			mState = NCS_Welcomed;
			inInputStream.Read( mPlayerId );
		}

		inInputStream.Read( kTimeBetweenStatePackets );
		kTimeBufferStatePackets = 4.f * kTimeBetweenStatePackets;

		//mReplicationManagerClient.Read( inInputStream );

		GEngine->AddOnScreenDebugMessage( -1, 6.f, FColor::Red,
			FString::Printf( TEXT( "%s    %d" ),
				*FString( "Welcome on client as PlayerID     = " ), mPlayerId )
		);

		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "****UDP**** HandleWelcomePacket Successfully!!!" );
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}
}

void NetworkMgr::HandleStatePacket( InputBitStream& inInputStream )
{
	if ( mState == NCS_Reseted )
	{
		ResetForNewGame();
		mState = NCS_Welcomed;
		mPlayerId = mResetedPlayerId;
	}
	if ( mState == NCS_Welcomed )
	{
		ReadLastMoveProcessedOnServerTimestamp( inInputStream );

		mReplicationManagerClient.Read( inInputStream );
	}
}

void NetworkMgr::ReadLastMoveProcessedOnServerTimestamp( InputBitStream& inInputStream )
{
	bool isTimestampDirty;
	inInputStream.Read( isTimestampDirty );
	if (isTimestampDirty)
	{
		inInputStream.Read( mLastMoveProcessedByServerTimestamp );

		float rtt = RealTimeSrvTiming::sInstance.GetFrameStartTime() - mLastMoveProcessedByServerTimestamp;
		mLastRoundTripTime = rtt;
		mAvgRoundTripTime.Update( rtt );

		InputMgr::sInstance->GetActionList().RemovedProcessedActions( mLastMoveProcessedByServerTimestamp );

		//A_LOG();
		//A_LOG_N( "rtt = ", rtt );
		//A_LOG_N( "ping = ", mAvgRoundTripTime.GetValue() / 2.f );

		float currentTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

		if ( currentTime > mTimeOfLastHello + kTimeBetweenHellos )
		{
			//A_MSG_M( 2.f, "ping = %f", mAvgRoundTripTime.GetValue() *1000.f );
			//GEngine->AddOnScreenDebugMessage( -1, 2.f, FColor::Red, FString::Printf( TEXT( "%s    %f" ), *FString( "ping" ), float( mAvgRoundTripTime.GetValue() *1000.f ) ) );

			GEngine->AddOnScreenDebugMessage( -1, 2.f, FColor::Red, 
				FString::Printf( TEXT( "%s    %f" ), 
				*FString( "ping" ), float( rtt *1000.f ) ) 
			);
			mTimeOfLastHello = currentTime;
		}
	}

}


void NetworkMgr::UpdateSendingInputPacket()
{
	float time = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	if (time > mTimeOfLastInputPacket + kTimeBetweenInputPackets)
	{
		SendInputPacket();
		mTimeOfLastInputPacket = time;
	}
}

void NetworkMgr::SendInputPacket()
{
	const ActionList& moveList = InputMgr::sInstance->GetActionList();

	if ( moveList.HasActions() )
	{
		OutputBitStream inputPacket;

		inputPacket.Write( kInputCC );

		mDeliveryNotificationManager.WriteState( inputPacket );

		int moveCount = moveList.GetActionCount();

		int firstMoveIndex = moveCount - 3;
		if ( firstMoveIndex < 3 )
		{
			firstMoveIndex = 0;
		}


		auto move = moveList.begin() + firstMoveIndex;


		inputPacket.Write( moveCount - firstMoveIndex, 2 );


		for ( ; firstMoveIndex < moveCount; ++firstMoveIndex, ++move )
		{
			move->Write( inputPacket );
		}

		SendPacket( inputPacket );
	}
}

RealTimeSrvEntityPtr NetworkMgr::GetGameObject( int inNetworkId ) const
{
	auto gameObjectIt = mNetworkIdToGameObjectMap.find( inNetworkId );
	if (gameObjectIt != mNetworkIdToGameObjectMap.end())
	{
		return gameObjectIt->second;
	}
	else
	{
		return RealTimeSrvEntityPtr();
	}
}

void NetworkMgr::AddToNetworkIdToGameObjectMap( RealTimeSrvEntityPtr inGameObject )
{
	mNetworkIdToGameObjectMap[inGameObject->GetNetworkId()] = inGameObject;
}

void NetworkMgr::RemoveFromNetworkIdToGameObjectMap( RealTimeSrvEntityPtr inGameObject )
{
	mNetworkIdToGameObjectMap.erase( inGameObject->GetNetworkId() );
}