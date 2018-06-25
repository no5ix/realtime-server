#pragma once

namespace realtime_srv
{

	class ReplicationMgr
	{
	public:
		ReplicationMgr( ClientProxy* clientProxy ) : owner_( clientProxy ) {}
	public:
		void ReplicateCreate( int inNetworkId, uint32_t inInitialDirtyState );
		void ReplicateDestroy( int inNetworkId );
		void SetReplicationStateDirty( int inNetworkId, uint32_t inDirtyState );
		void RemoveFromReplication( int inNetworkId );
		void HandleCreateAckd( int inNetworkId );

		void Write( OutputBitStream& inOutputStream, InFlightPacket* inInFlightPacket );

	private:

		uint32_t WriteCreateAction( OutputBitStream& inOutputStream,
			int inNetworkId, uint32_t inDirtyState );
		uint32_t WriteUpdateAction( OutputBitStream& inOutputStream,
			int inNetworkId, uint32_t inDirtyState );
		uint32_t WriteDestroyAction( OutputBitStream& inOutputStream,
			int inNetworkId, uint32_t inDirtyState );

	private:
		unordered_map< int, ReplicationCmd >	mNetworkIdToReplicationCommand;

		ClientProxy* owner_;
	};

}