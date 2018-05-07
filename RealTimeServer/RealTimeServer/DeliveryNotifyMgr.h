
class DeliveryNotifyMgr
{
public:
	
	
	DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks );
	~DeliveryNotifyMgr();
	
	inline	InFlightPacket*		WriteState( OutputBitStream& inOutputStream );
	inline	bool				ReadAndProcessState( InputBitStream& inInputStream );
	
	void				ProcessTimedOutPackets();
	
	uint32_t			GetDroppedPacketCount()		const	{ return mDroppedPacketCount; }
	uint32_t			GetDeliveredPacketCount()	const	{ return mDeliveredPacketCount; }
	uint32_t			GetDispatchedPacketCount()	const	{ return mDispatchedPacketCount; }
	
	const deque< InFlightPacket >&	GetInFlightPackets()	const	{ return mInFlightPackets; }
	
private:
	
	
	
	InFlightPacket*		WriteSequenceNumber( OutputBitStream& inOutputStream );
	void				WriteAckData( OutputBitStream& inOutputStream );
	
	bool				ProcessSequenceNumber( InputBitStream& inInputStream );
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

inline bool	DeliveryNotifyMgr::ReadAndProcessState( InputBitStream& inInputStream )
{
	bool toRet = ProcessSequenceNumber( inInputStream );
	if( mShouldProcessAcks )
	{
		ProcessAcks( inInputStream );
	}
	return toRet;
}