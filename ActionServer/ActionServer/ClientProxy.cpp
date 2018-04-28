#include "ActionServerPCH.h"


ClientProxy::ClientProxy( const SocketAddress& inSocketAddress, const string& inName, int inPlayerId ) :
	mSocketAddress( inSocketAddress ),
	mName( inName ),
	mPlayerId( inPlayerId ),
	mDeliveryNotificationManager( false, true ),
	mIsLastMoveTimestampDirty( false ),
	mTimeToRespawn( 0.f )
{
	UpdateLastPacketTime();
}


void ClientProxy::UpdateLastPacketTime()
{
	mLastPacketFromClientTime = Timing::sInstance.GetTimef();
}