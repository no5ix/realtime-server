#include "RealTimeSrvPCH.h"


ClientProxy::ClientProxy( const SocketAddrInterface& inSocketAddress, const string& inName, int inPlayerId, const UDPSocketPtr& inUDPSocket ) :
	mSocketAddress( inSocketAddress ),
	mName( inName ),
	mPlayerId( inPlayerId ),
	mUDPSocket( inUDPSocket ),
	mDeliveryNotificationManager( false, true ),
	mIsLastMoveTimestampDirty( false ),
	mTimeToRespawn( 0.f )
{
	UpdateLastPacketTime();
}


void ClientProxy::UpdateLastPacketTime()
{
	mLastPacketFromClientTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
}