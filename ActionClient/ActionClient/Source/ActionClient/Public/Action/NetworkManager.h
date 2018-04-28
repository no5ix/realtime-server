// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
//#include "ActionClient.h"
#include <memory>
#include <queue>
#include <list>
#include <unordered_map>
#include "Networking.h"
#include "MemoryBitStream.h"
#include "GameObjectRegistryUObj.h"
#include "ReplicationManagerClient.h"
#include "ActionEntity.h"
#include "DeliveryNotificationManager.h"
#include "WeightedTimedMovingAverage.h"


typedef std::unordered_map< int, GameObjectPtr > IntToGameObjectMap;

/**
 * 
 */
class NetworkManager
{
	enum NetworkClientState
	{
		NCS_Uninitialized,
		NCS_SayingHello,
		NCS_Welcomed
	};
public:
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';
	static const int		kMaxPacketsPerFrameCount = 10;

public:
	static std::unique_ptr<NetworkManager>	sInstance;

	static void StaticInit( const FString& inIP, int inPort, const FString& inPlayerName );
	
	void	SendPacket( const OutputMemoryBitStream& inOutputStream );

	void	ProcessIncomingPackets();

	void	ProcessPacket( InputMemoryBitStream& inInputStream );

	void	SendOutgoingPackets();

	const	WeightedTimedMovingAverage&		GetAvgRoundTripTime()	const { return mAvgRoundTripTime; }
	float									GetRoundTripTime()		const { return mAvgRoundTripTime.GetValue(); }

	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }
	int		GetPlayerId() const	{ return mPlayerId; }

	GameObjectPtr	GetGameObject( int inNetworkId ) const;
	void	AddToNetworkIdToGameObjectMap( GameObjectPtr inGameObject );
	void	RemoveFromNetworkIdToGameObjectMap( GameObjectPtr inGameObject );

	ReplicationManagerClient GetReplicationManagerClient() { return mReplicationManagerClient; }
	UGameObjectRegistryUObj* GetGameObjectRegistryUObj();
	void SetGameObjectRegistryUObj(UGameObjectRegistryUObj* inGameObjectRegistry, UWorld * inWorld);
protected:

	IntToGameObjectMap		mNetworkIdToGameObjectMap;
private:

	NetworkManager();

	void Init(
		const FString& inYourChosenSocketName,
		const FString& inIP,
		const int32 inPort,
		const FString& inPlayerName );


	void	HandleWelcomePacket( InputMemoryBitStream& inInputStream );

	void	UpdateBytesSentLastFrame();
	void	ReadIncomingPacketsIntoQueue();
	void	ProcessQueuedPackets();
	void	HandleStatePacket( InputMemoryBitStream& inInputStream );
	void	ReadLastMoveProcessedOnServerTimestamp( InputMemoryBitStream& inInputStream );

	void	UpdateSendingInputPacket();
	void    SendInputPacket();

	void    UpdateSayingHello();
	void    SendHelloPacket();

private:

	DeliveryNotificationManager	mDeliveryNotificationManager;
	ReplicationManagerClient	mReplicationManagerClient;

	UGameObjectRegistryUObj*    mGameObjectRegistryUObj;

	TSharedPtr<FInternetAddr>	mRemoteAddr;
	FSocket* mSocket;

	FString mPlayerName;

	NetworkClientState mState;

	float	mTimeOfLastHello;
	float				mTimeOfLastInputPacket;

	int mPlayerId;
	float				mLastMoveProcessedByServerTimestamp;
	float						mLastRoundTripTime;

	float						mDropPacketChance;
	float						mSimulatedLatency;

	WeightedTimedMovingAverage	mAvgRoundTripTime;

private:
	class ReceivedPacket
	{
	public:
		ReceivedPacket( float inReceivedTime, InputMemoryBitStream& inInputMemoryBitStream, const TSharedRef<FInternetAddr> inFromAddress ) : 
		    mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream )
		{
		}

		const	TSharedRef<FInternetAddr>&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputMemoryBitStream&	GetPacketBuffer()   { return mPacketBuffer; }

	private:

		float					mReceivedTime;
		InputMemoryBitStream	mPacketBuffer;
		TSharedRef<FInternetAddr>			mFromAddress;

	};

	std::queue< ReceivedPacket, std::list< ReceivedPacket > >	mPacketQueue;

};
