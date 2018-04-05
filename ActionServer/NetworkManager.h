//#pragma once
//#include "ActionServerShared.h"

class NetworkManager
{
public:
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';
	static const int		kMaxPacketsPerFrameCount = 10;

	NetworkManager();
	virtual ~NetworkManager();

	bool	Init( uint16_t inPort );
	void	ProcessIncomingPackets();

	virtual void	ProcessPacket( InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress ) {}
	virtual void HandleConnectionReset( const SocketAddress& inFromAddress ) {}

private:

	class ReceivedPacket
	{
	public:
		ReceivedPacket( float inReceivedTime, InputMemoryBitStream& inInputMemoryBitStream, const SocketAddress& inAddress );

		const	SocketAddress&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputMemoryBitStream&	GetPacketBuffer() { return mPacketBuffer; }

	private:

		float					mReceivedTime;
		InputMemoryBitStream	mPacketBuffer;
		SocketAddress			mFromAddress;

	};

	//void	UpdateBytesSentLastFrame();
	void	ReadIncomingPacketsIntoQueue();
	void	ProcessQueuedPackets();

	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;

	UDPSocketPtr	mSocket;

	//WeightedTimedMovingAverage	mBytesReceivedPerSecond;
	//WeightedTimedMovingAverage	mBytesSentPerSecond;

	int							mBytesSentThisFrame;

	float						mDropPacketChance;
	float						mSimulatedLatency;
};