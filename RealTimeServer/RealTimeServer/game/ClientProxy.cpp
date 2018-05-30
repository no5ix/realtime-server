#include "RealTimeSrvPCH.h"


ClientProxy::ClientProxy(
	const SocketAddrInterface& inSocketAddress,
	const std::string& inName,
	int inPlayerId,
	const UDPSocketPtr& inUDPSocket
	// ,
	// const std::shared_ptr<UdpConnection>& inUdpConnetction
) :
	mSocketAddress( inSocketAddress ),
	mName( inName ),
	mPlayerId( inPlayerId ),
	mUDPSocket( inUDPSocket ),
	mDeliveryNotificationManager( false, true ),
	mIsLastMoveTimestampDirty( false ),
	mTimeToRespawn( 0.f ),
	mRecvingServerResetFlag( false )
	// ,
	// UdpConnetction_( inUdpConnetction )
{
	UpdateLastPacketTime();
}


void ClientProxy::UpdateLastPacketTime()
{
	mLastPacketFromClientTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
}