
class ReplicationMgr;

class TransmissionDataHandler : public TransmissionData
{
public:

	TransmissionDataHandler( ReplicationMgr* inReplicationManagerServer ) :
	mReplicationManagerServer( inReplicationManagerServer )
	{}

	class ReplicationTransmission
	{
	public:
		ReplicationTransmission( int inNetworkId, ReplicationAction inAction, uint32_t inState ) :
		mNetworkId( inNetworkId ),
		mAction( inAction ),
		mState( inState )
		{}

		int							GetNetworkId()		const	{ return mNetworkId; }
		ReplicationAction		GetAction()			const	{ return mAction; }
		uint32_t					GetState()			const	{ return mState; }

	private:
		int							mNetworkId;
		ReplicationAction		mAction;
		uint32_t					mState;
	};

	void AddTransmission( int inNetworkId, ReplicationAction inAction, uint32_t inState );

	virtual void HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const override;
	virtual void HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const override;


private:

	void HandleCreateDeliveryFailure( int inNetworkId ) const;
	void HandleUpdateStateDeliveryFailure( int inNetworkId, uint32_t inState, DeliveryNotifyMgr* inDeliveryNotificationManager ) const;
	void HandleDestroyDeliveryFailure( int inNetworkId ) const;
	void HandleCreateDeliverySuccess( int inNetworkId ) const;
	void HandleDestroyDeliverySuccess( int inNetworkId ) const;

	ReplicationMgr*	mReplicationManagerServer;
	
	vector< ReplicationTransmission >		mTransmissions;

};

typedef shared_ptr< TransmissionDataHandler > TransmissionDataHandlerPtr;
