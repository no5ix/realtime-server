#include "ActionServerPCH.h"



NetworkManager::NetworkManager() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f )
{

}

NetworkManager::~NetworkManager()
{

}

bool NetworkManager::Init(uint16_t inPort)
{
#if _WIN32
	UDPSocket::StaticInit(); 
#endif

	mSocket = UDPSocket::CreateUDPSocket();
	SocketAddress ownAddress(INADDR_ANY, inPort);
	mSocket->Bind(ownAddress);

	LOG("Initializing NetworkManager at port %d", inPort);

	//mBytesReceivedPerSecond = WeightedTimedMovingAverage(1.f);
	//mBytesSentPerSecond = WeightedTimedMovingAverage(1.f);

	//did we bind okay?
	if (mSocket == nullptr)
	{
		return false;
	}

	if (mSocket->SetNonBlockingMode(true) != NO_ERROR)
	{
		return false;
	}

	return true;

}



void NetworkManager::ProcessIncomingPackets()
{
	ReadIncomingPacketsIntoQueue();

	ProcessQueuedPackets();

	//UpdateBytesSentLastFrame();

}

void NetworkManager::ReadIncomingPacketsIntoQueue()
{
	char packetMem[1500];
	int packetSize = sizeof( packetMem );
	InputMemoryBitStream inputStream( packetMem, packetSize * 8 );
	SocketAddress fromAddress;

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

			if (ActionServerMath::GetRandomFloat() >= mDropPacketChance)
			{
				float simulatedReceivedTime = Timing::sInstance.GetTimef() + mSimulatedLatency;
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

void NetworkManager::ProcessQueuedPackets()
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

void NetworkManager::SendPacket( const OutputMemoryBitStream& inOutputStream, const SocketAddress& inFromAddress )
{
	int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength(), inFromAddress );
	if (sentByteCount > 0)
	{
		mBytesSentThisFrame += sentByteCount;
	}
}

