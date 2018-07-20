
#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/game_obj/World.h"
#include "realtime_srv/rep/InflightPacket.h"
#include "realtime_srv/net/ClientProxy.h"

#include "realtime_srv/rep/ReplicationMgr.h"


using namespace realtime_srv;


void ReplicationMgr::Write( OutputBitStream& inOutputStream, InflightPacket* inInFlightPacket )
{
	for ( auto& pair : objIdToRepCmd_ )
	{
		ReplicationCmd& replicationCommand = pair.second;
		if ( replicationCommand.HasDirtyState() )
		{
			int objId = pair.first;
			inOutputStream.Write( objId );

			ReplicationAction action = replicationCommand.GetAction();
			inOutputStream.Write( action, 2 );

			uint32_t dirtyState = replicationCommand.GetDirtyState();

			uint32_t writtenState = 0;
			switch ( action )
			{
				case RA_Create:
					writtenState = WriteCreateAction( inOutputStream,
						objId, dirtyState );
					break;
				case RA_Update:
					writtenState = WriteUpdateAction( inOutputStream,
						objId, dirtyState );
					break;
				case RA_Destroy:
					writtenState = WriteDestroyAction( inOutputStream,
						objId, dirtyState );
					break;
				default:
					break;
			}

			inInFlightPacket->AddTransmission( objId, action, writtenState );
			replicationCommand.ClearDirtyState( writtenState );
			if ( inOutputStream.GetByteLength() > MAX_PACKET_BYTE_LENGTH )
				break;
		}
	}
}

uint32_t ReplicationMgr::WriteCreateAction( OutputBitStream& inOutputStream,
	int inObjId, uint32_t inDirtyState )
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inObjId );
	assert( gameObject );
	inOutputStream.Write( gameObject->GetClassId() );
	return gameObject->Write( inOutputStream, inDirtyState );
}

uint32_t ReplicationMgr::WriteUpdateAction( OutputBitStream& inOutputStream,
	int inObjId, uint32_t inDirtyState )
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inObjId );
	assert( gameObject );
	return gameObject->Write( inOutputStream, inDirtyState );
}

uint32_t ReplicationMgr::WriteDestroyAction( OutputBitStream& inOutputStream,
	int inObjId, uint32_t inDirtyState )
{
	( void )inOutputStream;
	( void )inObjId;
	( void )inDirtyState;

	return inDirtyState;
}