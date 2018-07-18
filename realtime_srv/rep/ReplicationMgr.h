#pragma once

namespace realtime_srv
{

class ReplicationMgr
{
public:
	ReplicationMgr( ClientProxy* clientProxy ) : owner_( clientProxy ) {}
public:
	void ReplicateCreate( int _objId, uint32_t inInitialDirtyState );
	void ReplicateDestroy( int _objId );
	void SetReplicationStateDirty( int _objId, uint32_t inDirtyState );
	void RemoveFromReplication( int _objId );
	void HandleCreateAckd( int _objId );

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