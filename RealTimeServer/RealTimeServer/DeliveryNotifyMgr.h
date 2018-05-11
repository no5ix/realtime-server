
class DeliveryNotifyMgr
{
public:
	
	
	DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks );
	~DeliveryNotifyMgr();
	
	inline	InFlightPacket*		WriteState( OutputBitStream& inOutputStream );
	inline	bool				ReadAndProcessState( InputBitStream& inInputStream, bool inIsSliced = false );
	
	void				ProcessTimedOutPackets();
	
	uint32_t			GetDroppedPacketCount()		const	{ return mDroppedPacketCount; }
	uint32_t			GetDeliveredPacketCount()	const	{ return mDeliveredPacketCount; }
	uint32_t			GetDispatchedPacketCount()	const	{ return mDispatchedPacketCount; }
	
	const deque< InFlightPacket >&	GetInFlightPackets()	const	{ return mInFlightPackets; }

public:
	static bool	SequenceGreaterThanOrEqual( PacketSequenceNumber s1, PacketSequenceNumber s2 );

	static bool SequenceGreaterThan( PacketSequenceNumber s1, PacketSequenceNumber s2 );
private:
	
	
	
	InFlightPacket*		WriteSequenceNumber( OutputBitStream& inOutputStream );
	void				WriteAckData( OutputBitStream& inOutputStream );
	
	bool				ProcessSequenceNumber( InputBitStream& inInputStream, bool inIsSliced = false );
	void				ProcessAcks( InputBitStream& inInputStream );
	
	
	void				AddPendingAck( PacketSequenceNumber inSequenceNumber );
	void				HandlePacketDeliveryFailure( const InFlightPacket& inFlightPacket );
	void				HandlePacketDeliverySuccess( const InFlightPacket& inFlightPacket );
	
	
	PacketSequenceNumber	mNextOutgoingSequenceNumber;
	PacketSequenceNumber	mNextExpectedSequenceNumber;
	
	deque< InFlightPacket >	mInFlightPackets;
	deque< AckRange >		mPendingAcks;
	
	bool					mShouldSendAcks;
	bool					mShouldProcessAcks;
	
	uint32_t		mDeliveredPacketCount;
	uint32_t		mDroppedPacketCount;
	uint32_t		mDispatchedPacketCount;
	
};



inline InFlightPacket* DeliveryNotifyMgr::WriteState( OutputBitStream& inOutputStream )
{
	InFlightPacket* toRet = WriteSequenceNumber( inOutputStream );
	if( mShouldSendAcks )
	{
		WriteAckData( inOutputStream );
	}
	return toRet;
}

inline bool DeliveryNotifyMgr::ReadAndProcessState( InputBitStream& inInputStream, bool inIsSliced/* = false*/ )
{
	bool toRet = ProcessSequenceNumber( inInputStream, inIsSliced );
	if ( mShouldProcessAcks )
	{
		ProcessAcks( inInputStream );
	}
	return toRet;
}

inline bool	DeliveryNotifyMgr::SequenceGreaterThanOrEqual( PacketSequenceNumber s1, PacketSequenceNumber s2 )
{
	return ( ( s1 >= s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
}

inline bool	DeliveryNotifyMgr::SequenceGreaterThan( PacketSequenceNumber s1, PacketSequenceNumber s2 )
{
	return ( ( s1 > s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
}