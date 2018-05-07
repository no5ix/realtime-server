#include "RealTimeServerPCH.h"



NetworkMgr::NetworkMgr() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mIsSimulatedJitter( false )
{

}

NetworkMgr::~NetworkMgr()
{

}

bool NetworkMgr::Init(uint16_t inPort)
{
#if _WIN32
	UDPSocketInterface::StaticInit(); 
#endif

	mSocket = UDPSocketInterface::CreateUDPSocket();

	//did we bind okay?
	if ( mSocket == nullptr )
	{
		return false;
	}

	SocketAddressInterface ownAddress(INADDR_ANY, inPort);
	mSocket->Bind(ownAddress);

	LOG("Initializing NetworkManager at port %d", inPort);

	//mBytesReceivedPerSecond = WeightedTimedMovingAverage(1.f);
	//mBytesSentPerSecond = WeightedTimedMovingAverage(1.f);


	if (mSocket->SetNonBlockingMode(true) != NO_ERROR)
	{
		return false;
	}

	return true;

}



void NetworkMgr::ProcessIncomingPackets()
{
	ReadIncomingPacketsIntoQueue();

	ProcessQueuedPackets();

	//UpdateBytesSentLastFrame();

}

void NetworkMgr::ReadIncomingPacketsIntoQueue()
{
	char packetMem[1500];
	int packetSize = sizeof( packetMem );
	InputBitStream inputStream( packetMem, packetSize * 8 );
	SocketAddressInterface fromAddress;

	int receivedPackedCount = 0;
	int totalReadByteCount = 0;

	while (receivedPackedCount < kMaxPacketsPerFrameCount)
	{
		int readByteCount = mSocket->ReceiveFrom( packetMem, packetSize, fromAddress );
		if (readByteCount == 0)
		{
			//LOG( "ReadIncomingPacketsIntoQueue readByteCount = %d ", 0 );
			break;
		}
		else if (readByteCount == -WSAECONNRESET)
		{
			HandleConnectionReset( fromAddress );
		}
		else if (readByteCount > 0)
		{
			inputStream.ResetToCapacity( readByteCount );
			++receivedPackedCount;
			totalReadByteCount += readByteCount;

			if (RealTimeSrvMath::GetRandomFloat() >= mDropPacketChance)
			{
				float simulatedReceivedTime =
					Timing::sInstance.GetTimef() +
					mSimulatedLatency +  
					( GetIsSimulatedJitter() ? 
						RealTimeSrvMath::Clamp( RealTimeSrvMath::GetRandomFloat(), 0.f, 0.06f ) : 0.f );
				mPacketQueue.emplace( simulatedReceivedTime, inputStream, fromAddress );
			}
			else
			{
				LOG( "Dropped packet!", 0 );
			}
		}
		else
		{
		}
	}

	if (totalReadByteCount > 0)
	{
		//mBytesReceivedPerSecond.UpdatePerSecond( static_cast< float >( totalReadByteCount ) );
	}
}

void NetworkMgr::ProcessQueuedPackets()
{
	//look at the front packet...
	while (!mPacketQueue.empty())
	{
		ReceivedPacket& nextPacket = mPacketQueue.front();
		if (Timing::sInstance.GetTimef() > nextPacket.GetReceivedTime())
		{
			ProcessPacket( nextPacket.GetPacketBuffer(), nextPacket.GetFromAddress() );
			mPacketQueue.pop();
		}
		else
		{
			break;
		}

	}

}

void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream, const SocketAddressInterface& inFromAddress )
{
	int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength(), inFromAddress );
	if (sentByteCount > 0)
	{
		mBytesSentThisFrame += sentByteCount;
	}
}

