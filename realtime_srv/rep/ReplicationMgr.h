#pragma once

namespace realtime_srv
{

class ReplicationMgr
{
public:
	ReplicationMgr( ClientProxy* clientProxy ) : owner_( clientProxy ) {}
public:

	void ReplicateCreate( int inObjId, uint32_t inInitDirtyState )
	{ objIdToRepCmd_[inObjId] = ReplicationCmd( inInitDirtyState ); }

	void ReplicateDestroy( int inObjId )
	{ objIdToRepCmd_[inObjId].SetDestroy(); }

	void RemoveFromReplication( int inObjId )
	{ objIdToRepCmd_.erase( inObjId ); }

	void SetReplicationStateDirty( int inObjId, uint32_t inDirtyState )
	{ objIdToRepCmd_[inObjId].AddDirtyState( inDirtyState ); }

	void HandleCreateAckd( int inObjId )
	{ objIdToRepCmd_[inObjId].HandleCreateAckd(); }

	void Write( OutputBitStream& inOutputStream, InFlightPacket* inInFlightPacket );

private:

	uint32_t WriteCreateAction( OutputBitStream& inOutputStream,
		int _objId, uint32_t inDirtyState );
	uint32_t WriteUpdateAction( OutputBitStream& inOutputStream,
		int _objId, uint32_t inDirtyState );
	uint32_t WriteDestroyAction( OutputBitStream& inOutputStream,
		int _objId, uint32_t inDirtyState );

private:
	unordered_map< int, ReplicationCmd >	objIdToRepCmd_;

	ClientProxy* owner_;
};

}