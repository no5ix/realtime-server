class DeliveryNotifyMgr;

class TransmissionData
{
public:
	virtual void HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const = 0;
	virtual void HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const = 0;
};
typedef shared_ptr< TransmissionData > TransmissionDataPtr;