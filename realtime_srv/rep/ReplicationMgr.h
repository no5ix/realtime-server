#pragma once

#include <unordered_map>
#include "realtime_srv/rep/ReplicationCmd.h"


namespace realtime_srv
{


class OutputBitStream;
class InflightPacket;
class ClientProxy;


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

	void Write( OutputBitStream& inOutputStream, InflightPacket* inInFlightPacket );

private:

	uint32_t WriteCreateAction( OutputBitStream& inOutputStream,
		int _objId, uint32_t inDirtyState );
	uint32_t WriteUpdateAction( OutputBitStream& inOutputStream,
		int _objId, uint32_t inDirtyState );
	uint32_t WriteDestroyAction( OutputBitStream& inOutputStream,
		int _objId, uint32_t inDirtyState );

private:
	std::unordered_map< int, ReplicationCmd >	objIdToRepCmd_;

	ClientProxy* owner_;
};

}