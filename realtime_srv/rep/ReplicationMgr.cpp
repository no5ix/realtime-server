#include "realtime_srv/common/RealtimeSrvShared.h"

using namespace realtime_srv;


void ReplicationMgr::ReplicateCreate( int inObjId, uint32_t inInitDirtyState )
{
	objIdToRepCmd_[inObjId] = ReplicationCmd( inInitDirtyState );
}

void ReplicationMgr::ReplicateDestroy( int inObjId )
{
	objIdToRepCmd_[inObjId].SetDestroy();
}

void ReplicationMgr::RemoveFromReplication( int inObjId )
{
	objIdToRepCmd_.erase( inObjId );
}

void ReplicationMgr::SetReplicationStateDirty( int inObjId, uint32_t inDirtyState )
{
	objIdToRepCmd_[inObjId].AddDirtyState( inDirtyState );
}

void ReplicationMgr::HandleCreateAckd( int inObjId )
{
	objIdToRepCmd_[inObjId].HandleCreateAckd();
}


void ReplicationMgr::Write( OutputBitStream& inOutputStream, InFlightPacket* inInFlightPacket )
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

			uint32_t writtenState = 0;
			uint32_t dirtyState = replicationCommand.GetDirtyState();

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
			{
				break;
			}
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