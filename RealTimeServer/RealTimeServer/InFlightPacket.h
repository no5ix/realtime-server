class DeliveryNotifyMgr;


class InFlightPacket
{
public:
	
	InFlightPacket( PacketSN inSequenceNumber );

	//~InFlightPacket()
	//{
	//	for ( const auto& pair : mTransmissionDataMap )
	//	{
	//		if ( --pair.second->mRefCount == 0 )
	//		{
	//			delete pair.second;
	//		}
	//	}
	//}
	
	PacketSN GetSequenceNumber() const	{ return mSequenceNumber; }
	float				 GetTimeDispatched() const	{ return mTimeDispatched; }
	
	void SetTransmissionData( int inKey, TransmissionDataPtr	inTransmissionData )
	{
		mTransmissionDataMap[ inKey ] = inTransmissionData;
		//++inTransmissionData->mRefCount;
	}
	const TransmissionDataPtr GetTransmissionData( int inKey ) const
	{
		auto it = mTransmissionDataMap.find( inKey );
		return ( it != mTransmissionDataMap.end() ) ? it->second : nullptr;
	}
	
	void HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const;
	void HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const;
	
private:
	PacketSN	mSequenceNumber;
	float					mTimeDispatched;
	
	unordered_map< int, TransmissionDataPtr >	mTransmissionDataMap;
};