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
	ReplicationMgr(ClientProxy* clientProxy) : owner_(clientProxy) {}
public:

	void ReplicateCreate(int objId, uint32_t initDirtyState)
	{ objIdToRepCmd_[objId] = ReplicationCmd(initDirtyState); }

	void ReplicateDestroy(int objId)
	{ objIdToRepCmd_[objId].SetDestroy(); }

	void RemoveFromReplication(int objId)
	{ objIdToRepCmd_.erase(objId); }

	void SetReplicationStateDirty(int objId, uint32_t dirtyState)
	{ objIdToRepCmd_[objId].AddDirtyState(dirtyState); }

	void HandleCreateAckd(int objId)
	{ objIdToRepCmd_[objId].HandleCreateAckd(); }

	void Write(OutputBitStream& outputStream, InflightPacket* inflightPkt);

private:

	uint32_t WriteCreateAction(OutputBitStream& outputStream,
		int objectId, uint32_t dirtyState);
	uint32_t WriteUpdateAction(OutputBitStream& outputStream,
		int objectId, uint32_t dirtyState);
	uint32_t WriteDestroyAction(OutputBitStream& outputStream,
		int objectId, uint32_t dirtyState);

private:
	std::unordered_map< int, ReplicationCmd >	objIdToRepCmd_;

	ClientProxy* owner_;
};

}