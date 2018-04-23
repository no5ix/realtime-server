// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "NetworkManager.h"
#include "ActionTiming.h"
#include "ActionSocketUtil.h"
#include "InputManager.h"
#include "ActionHelper.h"

std::unique_ptr<NetworkManager> NetworkManager::sInstance;

namespace
{
	const float kTimeBetweenHellos = 1.f;
	const float kTimeBetweenInputPackets = 0.033f;
}

NetworkManager::NetworkManager() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mState( NCS_Uninitialized )
{
	mSocket = NULL;
	//mReplicationManagerClient = NewObject<UReplicationManagerClientObj>();
	mGameObjectRegistryUObj = NewObject<UGameObjectRegistryUObj>();
}

void NetworkManager::StaticInit( const FString& inIP, int inPort, const FString& inPlayerName )
{
	sInstance.reset( new NetworkManager() );
	return sInstance->Init("ActionUDPSocket", inIP, inPort, inPlayerName );
}

void NetworkManager::Init( const FString& inYourChosenSocketName, const FString& inIP, const int32 inPort, const FString& inPlayerName )
{
	mPlayerName = inPlayerName;

	ActionSocketUtil::CreateUDPSocket( mSocket, inYourChosenSocketName );

	if ( ActionSocketUtil::CreateInternetAddress( mRemoteAddr, inIP, inPort ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Warning, TEXT( "ActionUDPSocket Initialized Successfully!!!" ) );
		UE_LOG( LogTemp, Warning, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
	}

	mState = NCS_SayingHello;
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
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Warning, TEXT( "****UDP**** Send Hello Packet Successfully!!!" ) );
		UE_LOG( LogTemp, Warning, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
	}
}

void NetworkManager::SendHelloPacket()
{
	OutputMemoryBitStream helloPacket;

	helloPacket.Write( kHelloCC );
	helloPacket.Write( std::string( TCHAR_TO_UTF8( *mPlayerName ) ) );

	SendPacket( helloPacket );
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


	static bool isFirstFlagForDebug = false;
	if (!isFirstFlagForDebug)
	{

		UE_LOG( LogTemp, Warning, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Warning, TEXT( "****UDP**** DataRecv come in!!!" ) );
		UE_LOG( LogTemp, Warning, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

		isFirstFlagForDebug = true;
	}


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
			float simulatedReceivedTime = ActionTiming::sInstance.GetTimef() + mSimulatedLatency;
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
		if (ActionTiming::sInstance.GetTimef() > nextPacket.GetReceivedTime())
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
		//if (mDeliveryNotificationManager.ReadAndProcessState( inInputStream ))
		//{

		UE_LOG( LogTemp, Warning, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Warning, TEXT( "****HandleStatePacket;****  Successfully!!!" ) );
		UE_LOG( LogTemp, Warning, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

			HandleStatePacket( inInputStream );
		//}
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
		//UpdateSendingInputPacket();
		break;
	}
}

void NetworkManager::UpdateSayingHello()
{
	float currentTime = ActionTiming::sInstance.GetTimef();

	if (currentTime > mTimeOfLastHello + kTimeBetweenHellos)
	{
		SendHelloPacket();
		mTimeOfLastHello = currentTime;
	}
}

UGameObjectRegistryUObj* NetworkManager::GetGameObjectRegistryUObj()
{
	if (mGameObjectRegistryUObj == nullptr)
	{
		mGameObjectRegistryUObj = NewObject<UGameObjectRegistryUObj>();
		if (mGameObjectRegistryUObj)
		{
			ActionHelper::OutputLog( "mGameObjectRegistryUObj is not null" );
		}
		else
		{
			ActionHelper::OutputLog( "mGameObjectRegistryUObj is null" );
		}
	}
	return mGameObjectRegistryUObj;
}

void NetworkManager::HandleWelcomePacket( InputMemoryBitStream& inInputStream )
{
	if (mState == NCS_SayingHello)
	{
		//if we got a player id, we've been welcomed!

		inInputStream.Read( mPlayerId );
		mState = NCS_Welcomed;


		ActionHelper::ScreenMsg( "welcome on client as playerID = ", mPlayerId );

		UE_LOG( LogTemp, Warning, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Warning, TEXT( "****UDP**** HandleWelcomePacket Successfully!!!" ) );
		UE_LOG( LogTemp, Warning, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

	}
}

void NetworkManager::HandleStatePacket( InputMemoryBitStream& inInputStream )
{
	if (mState == NCS_Welcomed)
	{
		ReadLastMoveProcessedOnServerTimestamp( inInputStream );

		UE_LOG( LogTemp, Warning, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Warning, TEXT( "****ReadLastMoveProcessedOnServerTimestamp;****  Successfully!!!" ) );
		UE_LOG( LogTemp, Warning, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

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
		//mAvgRoundTripTime.Update( rtt );

		InputManager::sInstance->GetActionList().RemovedProcessedActions( mLastMoveProcessedByServerTimestamp );

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