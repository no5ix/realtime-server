class DeliveryNotifyMgr;

class TransmissionData
{
public:
	TransmissionData() : mRefCount( 0 ) {}
	virtual ~TransmissionData() {}

	virtual void HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const = 0;
	virtual void HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const = 0;

	//int&		GetRefCount() { return mRefCount; }

//private:
	int mRefCount;
};
typedef shared_ptr< TransmissionData > TransmissionDataPtr;