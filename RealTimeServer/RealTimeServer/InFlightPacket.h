class DeliveryNotifyMgr;

typedef uint16_t	PacketSequenceNumber;

class InFlightPacket
{
public:
	
	InFlightPacket( PacketSequenceNumber inSequenceNumber );
	
	PacketSequenceNumber GetSequenceNumber() const	{ return mSequenceNumber; }
	float				 GetTimeDispatched() const	{ return mTimeDispatched; }
	
	void 				 SetTransmissionData( int inKey, TransmissionDataPtr	inTransmissionData )
	{
		mTransmissionDataMap[ inKey ] = inTransmissionData;
	}
	const TransmissionDataPtr GetTransmissionData( int inKey ) const
	{
		auto it = mTransmissionDataMap.find( inKey );
		return ( it != mTransmissionDataMap.end() ) ? it->second : nullptr;
	}
	
	void			HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const;
	void			HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const;
	
private:
	PacketSequenceNumber	mSequenceNumber;
	float			mTimeDispatched;
	
	unordered_map< int, TransmissionDataPtr >	mTransmissionDataMap;
};