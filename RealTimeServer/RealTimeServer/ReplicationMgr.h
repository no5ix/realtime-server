#pragma once


class ReplicationMgr
{


public:
	void ReplicateCreate( int inNetworkId, uint32_t inInitialDirtyState );
	void ReplicateDestroy( int inNetworkId );
	void SetStateDirty( int inNetworkId, uint32_t inDirtyState );
	void RemoveFromReplication( int inNetworkId );
	void HandleCreateAckd( int inNetworkId );

	void Write( OutputBitStream& inOutputStream, TransmissionDataHandler* inTransmissinData );

private:

	uint32_t WriteCreateAction( OutputBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState );
	uint32_t WriteUpdateAction( OutputBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState );
	uint32_t WriteDestroyAction( OutputBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState );

	unordered_map< int, ReplicationCmd >	mNetworkIdToReplicationCommand;


};