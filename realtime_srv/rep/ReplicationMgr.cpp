#include "realtime_srv/common/RealtimeSrvShared.h"

using namespace realtime_srv;


void ReplicationMgr::ReplicateCreate( int inNetId, uint32_t inInitDirtyState )
{
	mNetIdToRepCmd[inNetId] = ReplicationCmd( inInitDirtyState );
}

void ReplicationMgr::ReplicateDestroy( int inNetId )
{
	mNetIdToRepCmd[inNetId].SetDestroy();
}

void ReplicationMgr::RemoveFromReplication( int inNetId )
{
	mNetIdToRepCmd.erase( inNetId );
}

void ReplicationMgr::SetReplicationStateDirty( int inNetId, uint32_t inDirtyState )
{
	mNetIdToRepCmd[inNetId].AddDirtyState( inDirtyState );
}

void ReplicationMgr::HandleCreateAckd( int inNetId )
{
	mNetIdToRepCmd[inNetId].HandleCreateAckd();
}


void ReplicationMgr::Write( OutputBitStream& inOutputStream, InFlightPacket* inInFlightPacket )
{
	for ( auto& pair : mNetIdToRepCmd )
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
	int inNetId, uint32_t inDirtyState )
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inNetId );

	inOutputStream.Write( gameObject->GetClassId() );
	return gameObject->Write( inOutputStream, inDirtyState );
}

uint32_t ReplicationMgr::WriteUpdateAction( OutputBitStream& inOutputStream,
	int inNetId, uint32_t inDirtyState )
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inNetId );

	uint32_t writtenState = gameObject->Write( inOutputStream, inDirtyState );

	return writtenState;
}

uint32_t ReplicationMgr::WriteDestroyAction( OutputBitStream& inOutputStream,
	int inNetId, uint32_t inDirtyState )
{
	( void )inOutputStream;
	( void )inNetId;
	( void )inDirtyState;


	return inDirtyState;
}