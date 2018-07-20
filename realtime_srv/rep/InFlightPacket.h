#pragma once

#include <unordered_map>
#include "realtime_srv/common/RealtimeSrvMacro.h"
#include "realtime_srv/rep/ReplicationCmd.h"


namespace realtime_srv
{


class DeliveryNotifyMgr;
class ReplicationMgr;
class ClientProxy;


class InflightPacket
{
public:
	InflightPacket( PacketSN inSequenceNumber, ClientProxy* inClientProxy );

	PacketSN GetSequenceNumber() const { return mSequenceNumber; }
	float GetTimeDispatched() const { return mTimeDispatched; }
public:

	class ReplicationTransmission
	{
	public:
		ReplicationTransmission( int inObjId,
			ReplicationAction inAction, uint32_t inState )
			:
			ObjId_( inObjId ),
			mAction( inAction ),
			mState( inState )
		{}

		int							GetObjId()		const { return ObjId_; }
		ReplicationAction			GetAction()			const { return mAction; }
		uint32_t					GetState()			const { return mState; }

	private:
		int							ObjId_;
		ReplicationAction			mAction;
		uint32_t					mState;
	};

	void AddTransmission( int inObjId, ReplicationAction inAction, uint32_t inState );

	virtual void HandleDeliveryFailure() const;
	virtual void HandleDeliverySuccess( ) const;


private:

	void HandleCreateDeliveryFailure( int inObjId ) const;
	void HandleUpdateStateDeliveryFailure( int inObjId, 
		uint32_t inState ) const;

	void HandleDestroyDeliveryFailure( int inObjId ) const
	{ owner_->GetReplicationMgr().ReplicateDestroy( inObjId ); }

	void HandleCreateDeliverySuccess( int inObjId ) const
	{ owner_->GetReplicationMgr().HandleCreateAckd( inObjId ); }

	void HandleDestroyDeliverySuccess( int inObjId ) const
	{ owner_->GetReplicationMgr().RemoveFromReplication( inObjId ); }


private:
	std::unordered_map< int, ReplicationTransmission >	objIdToTransMap_;

	PacketSN	mSequenceNumber;
	float		mTimeDispatched;
	ClientProxy* owner_;
};

}