#include "realtime_srv/common/RealtimeSrvShared.h"

using namespace realtime_srv;


void ReplicationMgr::ReplicateCreate( int inObjId, uint32_t inInitDirtyState )
{
	mObjIdToRepCmd[inObjId] = ReplicationCmd( inInitDirtyState );
}

void ReplicationMgr::ReplicateDestroy( int inObjId )
{
	mObjIdToRepCmd[inObjId].SetDestroy();
}

void ReplicationMgr::RemoveFromReplication( int inObjId )
{
	mObjIdToRepCmd.erase( inObjId );
}

void ReplicationMgr::SetReplicationStateDirty( int inObjId, uint32_t inDirtyState )
{
	mObjIdToRepCmd[inObjId].AddDirtyState( inDirtyState );
}

void ReplicationMgr::HandleCreateAckd( int inObjId )
{
	mObjIdToRepCmd[inObjId].HandleCreateAckd();
}


void ReplicationMgr::Write( OutputBitStream& inOutputStream, InFlightPacket* inInFlightPacket )
{
	for ( auto& pair : mObjIdToRepCmd )
	{
		ReplicationCmd& replicationCommand = pair.second;
		if ( replicationCommand.HasDirtyState() )
		{
			int networkId = pair.first;

			inOutputStream.Write( networkId );

			ReplicationAction action = replicationCommand.GetAction();
			inOutputStream.Write( action, 2 );

			uint32_t writtenState = 0;
			uint32_t dirtyState = replicationCommand.GetDirtyState();

			switch ( action )
			{
				case RA_Create:
					writtenState = WriteCreateAction( inOutputStream,
						networkId, dirtyState );
					break;
				case RA_Update:
					writtenState = WriteUpdateAction( inOutputStream,
						networkId, dirtyState );
					break;
				case RA_Destroy:
					writtenState = WriteDestroyAction( inOutputStream,
						networkId, dirtyState );
					break;
				default:
					break;
			}

			inInFlightPacket->AddTransmission( networkId, action, writtenState );

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

	inOutputStream.Write( gameObject->GetClassId() );
	return gameObject->Write( inOutputStream, inDirtyState );
}

uint32_t ReplicationMgr::WriteUpdateAction( OutputBitStream& inOutputStream,
	int inObjId, uint32_t inDirtyState )
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inObjId );

	uint32_t writtenState = gameObject->Write( inOutputStream, inDirtyState );

	return writtenState;
}

uint32_t ReplicationMgr::WriteDestroyAction( OutputBitStream& inOutputStream,
	int inObjId, uint32_t inDirtyState )
{
	( void )inOutputStream;
	( void )inObjId;
	( void )inDirtyState;


	return inDirtyState;
}