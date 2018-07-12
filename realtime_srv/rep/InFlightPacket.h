#pragma once

namespace realtime_srv
{


class DeliveryNotifyMgr;
class ReplicationMgr;

class InFlightPacket
{
public:
	InFlightPacket( PacketSN inSequenceNumber, ClientProxy* inClientProxy );

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
	void HandleDestroyDeliveryFailure( int inObjId ) const;
	void HandleCreateDeliverySuccess( int inObjId ) const;
	void HandleDestroyDeliverySuccess( int inObjId ) const;

private:
	std::unordered_map< int, ReplicationTransmission >	NetIdToTransMap_;

	PacketSN	mSequenceNumber;
	float		mTimeDispatched;
	ClientProxy* owner_;
};

}