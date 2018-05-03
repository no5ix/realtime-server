class DeliveryNotificationMgr;

class TransmissionData
{
public:
	virtual void HandleDeliveryFailure( DeliveryNotificationMgr* inDeliveryNotificationManager ) const = 0;
	virtual void HandleDeliverySuccess( DeliveryNotificationMgr* inDeliveryNotificationManager ) const = 0;
};
typedef shared_ptr< TransmissionData > TransmissionDataPtr;