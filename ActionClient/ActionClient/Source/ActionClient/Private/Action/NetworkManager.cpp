// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "NetworkManager.h"
#include "ActionTiming.h"
#include "ActionSocketUtil.h"
#include "InputManager.h"
#include "ActionHelper.h"
#include "ActionList.h"

std::unique_ptr<NetworkManager> NetworkManager::sInstance;

float NetworkManager::kTimeBufferStatePackets = 0.132f;
float NetworkManager::kTimeBetweenStatePackets = 0.033f;

namespace
{
	const float kTimeBetweenHellos = 1.f;
	const float kTimeBetweenInputPackets = 0.033f;
}

NetworkManager::NetworkManager() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mPlayerId(-1.f),
	mDeliveryNotificationManager( true, false ),
	mState( NCS_Uninitialized )
{
	mSocket = NULL;
	//mReplicationManagerClient = NewObject<UReplicationManagerClientObj>();
}

void NetworkManager::StaticInit( const FString& inIP, int inPort, const FString& inPlayerName )
{
	sInstance.reset( new NetworkManager() );
	return sInstance->Init("ActionUDPSocket", inIP, inPort, inPlayerName );
}

void NetworkManager::Init( const FString& inYourChosenSocketName, const FString& inIP, const int32 inPort, const FString& inPlayerName )
{
	mGameObjectRegistryUObj = NewObject<UGameObjectRegistryUObj>();

	mPlayerName = inPlayerName;

	ActionSocketUtil::CreateUDPSocket( mSocket, inYourChosenSocketName );

	if ( ActionSocketUtil::CreateInternetAddress( mRemoteAddr, inIP, inPort ) )
	{
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "ActionUDPSocket Initialized Successfully!!!" );
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}

	mState = NCS_SayingHello;

	mAvgRoundTripTime = WeightedTimedMovingAverage( 1.f );
}


void NetworkManager::SendPacket( const OutputMemoryBitStream& inOutputStream )
{
	int32 BytesSent = 0;
	ActionSocketUtil::SendTo( mSocket, inOutputStream, BytesSent, mRemoteAddr );

	if (BytesSent <= 0)
	{
		const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *Str );
		ActionHelper::ScreenMsg( Str );
	}
}

void NetworkManager::SendHelloPacket()
{
	OutputMemoryBitStream helloPacket;

	helloPacket.Write( kHelloCC );
	helloPacket.Write( std::string( TCHAR_TO_UTF8( *mPlayerName ) ) );

	SendPacket( helloPacket );
	
	
	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	A_LOG_1( "****UDP**** Send Hello Packet Successfully!!!" );
	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	
}


void NetworkManager::ProcessIncomingPackets()
{
	ReadIncomingPacketsIntoQueue();

	ProcessQueuedPackets();

	UpdateBytesSentLastFrame();
}

void NetworkManager::ReadIncomingPacketsIntoQueue()
{

	char ReceivedDataPacketMem[1500];
	int32 packetSize = sizeof( ReceivedDataPacketMem );
	InputMemoryBitStream inputStream( ReceivedDataPacketMem, packetSize * 8 );
	int32 refReadByteCount = 0;

	TSharedRef<FInternetAddr> fromAddress = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->CreateInternetAddr();




	//keep reading until we don't have anything to read ( or we hit a max number that we'll process per frame )
	int receivedPackedCount = 0;

	while (receivedPackedCount < kMaxPacketsPerFrameCount)
	{
		ActionSocketUtil::RecvFrom( mSocket, ReceivedDataPacketMem, packetSize, refReadByteCount, fromAddress );

		if (refReadByteCount == 0)
		{
			//nothing to read
			break;
		}
		else if (refReadByteCount > 0)
		{
			inputStream.ResetToCapacity( refReadByteCount );
			++receivedPackedCount;

			//UGameplayStatics::GetRealTimeSeconds(GetWorld());
			float simulatedReceivedTime = ActionTiming::sInstance.GetCurrentGameTime() + mSimulatedLatency;
			mPacketQueue.emplace( simulatedReceivedTime, inputStream, fromAddress );
			//FDateTime::ToUnixTimestamp()
		}
		else
		{
		}
	}
	
}



void NetworkManager::ProcessQueuedPackets()
{
	//look at the front packet...
	while (!mPacketQueue.empty())
	{
		ReceivedPacket& nextPacket = mPacketQueue.front();
		if (ActionTiming::sInstance.GetCurrentGameTime() > nextPacket.GetReceivedTime())
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

void NetworkManager::UpdateBytesSentLastFrame()
{

}


void NetworkManager::ProcessPacket( InputMemoryBitStream& inInputStream )
{
	uint32_t	packetType;
	inInputStream.Read( packetType );
	switch (packetType)
	{
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


void NetworkManager::SendOutgoingPackets()
{
	switch (mState)
	{
	case NCS_SayingHello:
		UpdateSayingHello();
		break;
	case NCS_Welcomed:
		UpdateSendingInputPacket();
		break;
	}
}

void NetworkManager::UpdateSayingHello()
{
	float currentTime = ActionTiming::sInstance.GetCurrentGameTime();

	if (currentTime > mTimeOfLastHello + kTimeBetweenHellos)
	{
		SendHelloPacket();
		mTimeOfLastHello = currentTime;
	}
}

UGameObjectRegistryUObj* NetworkManager::GetGameObjectRegistryUObj()
{
	//if (mGameObjectRegistryUObj == nullptr)
	//{
		//mGameObjectRegistryUObj = NewObject<UGameObjectRegistryUObj>();
		//if (mGameObjectRegistryUObj)
		//{
		//	ActionHelper::OutputLog( "mGameObjectRegistryUObj is not null" );
		//}
		//else
		//{
		//	ActionHelper::OutputLog( "mGameObjectRegistryUObj is null" );
		//}
	//}
	return mGameObjectRegistryUObj;
}

void NetworkManager::SetGameObjectRegistryUObj( UGameObjectRegistryUObj* inGameObjectRegistry, UWorld * inWorld )
{
	mGameObjectRegistryUObj = inGameObjectRegistry;
	mGameObjectRegistryUObj->World = inWorld;
}

void NetworkManager::HandleWelcomePacket( InputMemoryBitStream& inInputStream )
{
	if (mState == NCS_SayingHello)
	{
		//if we got a player id, we've been welcomed!

		inInputStream.Read( mPlayerId );
		mState = NCS_Welcomed;


		inInputStream.Read( kTimeBetweenStatePackets );
		kTimeBufferStatePackets = 4.f * kTimeBetweenStatePackets;

		mReplicationManagerClient.Read( inInputStream );

		ActionHelper::ScreenMsg( "welcome on client as playerID = ", mPlayerId );

		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "****UDP**** HandleWelcomePacket Successfully!!!" );
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	}
}

void NetworkManager::HandleStatePacket( InputMemoryBitStream& inInputStream )
{
	if (mState == NCS_Welcomed)
	{
		ReadLastMoveProcessedOnServerTimestamp( inInputStream );

		//A_LOG();

		//old
		//HandleGameObjectState( inPacketBuffer );
		//HandleScoreBoardState( inInputStream );


		//tell the replication manager to handle the rest...
		mReplicationManagerClient.Read( inInputStream );
	}
}

void NetworkManager::ReadLastMoveProcessedOnServerTimestamp( InputMemoryBitStream& inInputStream )
{
	bool isTimestampDirty;
	inInputStream.Read( isTimestampDirty );
	if (isTimestampDirty)
	{
		inInputStream.Read( mLastMoveProcessedByServerTimestamp );

		float rtt = ActionTiming::sInstance.GetFrameStartTime() - mLastMoveProcessedByServerTimestamp;
		mLastRoundTripTime = rtt;
		mAvgRoundTripTime.Update( rtt );

		InputManager::sInstance->GetActionList().RemovedProcessedActions( mLastMoveProcessedByServerTimestamp );

		//A_LOG();
		//A_LOG_N( "rtt = ", rtt );
		//A_LOG_N( "ping = ", mAvgRoundTripTime.GetValue() / 2.f );

		float currentTime = ActionTiming::sInstance.GetCurrentGameTime();

		if ( currentTime > mTimeOfLastHello + kTimeBetweenHellos )
		{
			//A_MSG_M( 2.f, "ping = %f", mAvgRoundTripTime.GetValue() *1000.f );
			//GEngine->AddOnScreenDebugMessage( -1, 2.f, FColor::Red, FString::Printf( TEXT( "%s    %f" ), *FString( "ping" ), float( mAvgRoundTripTime.GetValue() *1000.f ) ) );
			GEngine->AddOnScreenDebugMessage( -1, 2.f, FColor::Red, FString::Printf( TEXT( "%s    %f" ), *FString( "ping" ), float( rtt *1000.f ) ) );
			mTimeOfLastHello = currentTime;
		}
	}

}


void NetworkManager::UpdateSendingInputPacket()
{
	float time = ActionTiming::sInstance.GetCurrentGameTime();

	if (time > mTimeOfLastInputPacket + kTimeBetweenInputPackets)
	{
		SendInputPacket();
		mTimeOfLastInputPacket = time;
	}
}

void NetworkManager::SendInputPacket()
{
	const ActionList& moveList = InputManager::sInstance->GetActionList();

	if ( moveList.HasActions() )
	{
		OutputMemoryBitStream inputPacket;

		inputPacket.Write( kInputCC );

		mDeliveryNotificationManager.WriteState( inputPacket );

		int moveCount = moveList.GetActionCount();

		int firstMoveIndex = moveCount - 3;
		if ( firstMoveIndex < 3 )
		{
			firstMoveIndex = 0;
		}

		// 		int firstMoveIndex = moveCount > 3 ? moveCount - 3 - 1 : 0;

		//int hlhTestfirstMoveIndex = firstMoveIndex;

		auto move = moveList.begin() + firstMoveIndex;


		//only need two bits to write the move count, because it's 0, 1, 2 or 3
		inputPacket.Write( moveCount - firstMoveIndex, 2 );

		//int hlhTestSendTimes = 0;

		for ( ; firstMoveIndex < moveCount; ++firstMoveIndex, ++move )
		{
			///would be nice to optimize the time stamp...
			move->Write( inputPacket );
			//hlhTestSendTimes++;
		}

		// 		if ( hlhTestSendTimes > 3 )
		// 		{
		//A_LOG_M( "moveCount = %d, hlhTestfirstMoveIndex = %d, hlhTestSendTimes = %d", moveCount, hlhTestfirstMoveIndex, hlhTestSendTimes );
		// 		}

		SendPacket( inputPacket );
	}
}

GameObjectPtr NetworkManager::GetGameObject( int inNetworkId ) const
{
	auto gameObjectIt = mNetworkIdToGameObjectMap.find( inNetworkId );
	if (gameObjectIt != mNetworkIdToGameObjectMap.end())
	{
		return gameObjectIt->second;
	}
	else
	{
		return GameObjectPtr();
	}
}

void NetworkManager::AddToNetworkIdToGameObjectMap( GameObjectPtr inGameObject )
{
	mNetworkIdToGameObjectMap[inGameObject->GetNetworkId()] = inGameObject;
}

void NetworkManager::RemoveFromNetworkIdToGameObjectMap( GameObjectPtr inGameObject )
{
	mNetworkIdToGameObjectMap.erase( inGameObject->GetNetworkId() );
}