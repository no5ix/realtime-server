// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
//
#include <memory>
#include <queue>
#include <list>
#include <unordered_map>
#include "Networking.h"
#include "BitStream.h"
#include "ReplicationMgr.h"
#include "RealTimeSrvEntity.h"
#include "DeliveryNotifyMgr.h"
#include "WeightedTimedMovingAverage.h"


typedef std::unordered_map< int, RealTimeSrvEntityPtr > IntToGameObjectMap;

/**
 * 
 */
class NetworkMgr
{
	enum NetworkClientState
	{
		NCS_Uninitialized,
		NCS_Resetting,
		NCS_Reseted,
		NCS_SayingHello,
		NCS_Welcomed
	};
public:
	static const uint32_t	kHelloCC	= 'HELO';
	static const uint32_t	kWelcomeCC	= 'WLCM';
	static const uint32_t	kResetCC	= 'RSET';
	static const uint32_t	kResetedCC	= 'RSTD';
	static const uint32_t	kStateCC	= 'STAT';
	static const uint32_t	kInputCC	= 'INPT';

	static const int		kMaxPacketsPerFrameCount = 10;

	static float kTimeBufferStatePackets;
	static float kTimeBetweenStatePackets;

public:
	static std::unique_ptr<NetworkMgr>	sInstance;

	static void StaticInit( const FString& inIP, int inPort, const FString& inPlayerName );
	
	void	Update();

	void	SendPacket( const OutputBitStream& inOutputStream );

	void	ProcessIncomingPackets();

	void	ProcessPacket( InputBitStream& inInputStream );

	void	SendOutgoingPackets();

	const	WeightedTimedMovingAverage&		GetAvgRoundTripTime()	const { return mAvgRoundTripTime; }
	float									GetRoundTripTime()		const { return mAvgRoundTripTime.GetValue(); }

	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }
	int		GetPlayerId() const	{ return mPlayerId; }

	RealTimeSrvEntityPtr	GetGameObject( int inNetworkId ) const;
	void	AddToNetworkIdToGameObjectMap( RealTimeSrvEntityPtr inGameObject );
	void	RemoveFromNetworkIdToGameObjectMap( RealTimeSrvEntityPtr inGameObject );

	ReplicationMgr GetReplicationManagerClient() { return mReplicationManagerClient; }

	void	UpdateLastPacketFromSrvTime();
	float	GetLastPacketFromClientTime()	const { return mLastPacketFromSrvTime; }
	void	CheckForDisconnects();
	void	ResetForNewGame();

//	bool	GetRecvingServerResetFlag() const { return mRecvingServerResetFlag; }
//	bool	SetRecvingServerResetFlag( bool inRecvingServerResetFlag ) { mRecvingServerResetFlag = inRecvingServerResetFlag; }
//
//private:
//	bool			mRecvingServerResetFlag;

protected:

	IntToGameObjectMap		mNetworkIdToGameObjectMap;
private:

	NetworkMgr();

	void Init(
		const FString& inYourChosenSocketName,
		const FString& inIP,
		const int32 inPort,
		const FString& inPlayerName );

	void	HandleResetPacket();
	void	HandleWelcomePacket( InputBitStream& inInputStream );

	void	UpdateBytesSentLastFrame();

	// not complete, deprecated. can not calc slicedPacketCount.
	void	RecombineSlicesToChunk( InputBitStream& inInputStream );

	void	ReadIncomingPacketsIntoQueue();
	void	ProcessQueuedPackets();
	void	HandleStatePacket( InputBitStream& inInputStream );
	void	ReadLastMoveProcessedOnServerTimestamp( InputBitStream& inInputStream );

	void	UpdateSendingInputPacket();
	void    SendInputPacket();

	void    UpdateSayingHello();
	void    SendHelloPacket();

private:

	DeliveryNotifyMgr			mDeliveryNotificationManager;
	ReplicationMgr				mReplicationManagerClient;


	TSharedPtr<FInternetAddr>	mRemoteAddr;
	FSocket*					mSocket;

	FString						mPlayerName;

	NetworkClientState			mState;

	float						mTimeOfLastHello;
	float						mTimeOfLastInputPacket;

	int							mPlayerId;
	int							mResetedPlayerId;
	float						mLastMoveProcessedByServerTimestamp;
	float						mLastRoundTripTime;

	float						mDropPacketChance;
	float						mSimulatedLatency;

	WeightedTimedMovingAverage	mAvgRoundTripTime;

	float						mLastCheckDCTime;
	float						mLastPacketFromSrvTime;

	bool						mIsReceivingSlicePacket;
	uint8_t						mNextExpectedSlicedPacketIndex;

	InputBitStream				mChunkInputStream;
	ChunkPacketID				mChunkPacketID;

private:
	class ReceivedPacket
	{
	public:
		ReceivedPacket( float inReceivedTime, InputBitStream& inInputMemoryBitStream, const TSharedRef<FInternetAddr> inFromAddress ) : 
		    mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream )
		{
		}

		const	TSharedRef<FInternetAddr>&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer()   { return mPacketBuffer; }

	private:

		float					mReceivedTime;
		InputBitStream	mPacketBuffer;
		TSharedRef<FInternetAddr>			mFromAddress;

	};

	std::queue< ReceivedPacket, std::list< ReceivedPacket > >	mPacketQueue;

};
