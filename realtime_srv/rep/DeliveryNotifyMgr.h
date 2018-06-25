#pragma once

namespace realtime_srv
{


	class ClientProxy;

	class DeliveryNotifyMgr
	{
	public:


		DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks );
		~DeliveryNotifyMgr();

		inline	InFlightPacket*		WriteState( OutputBitStream& inOutputStream,
			ClientProxy* inClientProxy );
		inline bool					ReadAndProcessState( InputBitStream& inInputStream );

		void						ProcessTimedOutPackets();

		uint32_t					GetDroppedPacketCount()		const { return mDroppedPacketCount; }
		uint32_t					GetDeliveredPacketCount()	const { return mDeliveredPacketCount; }
		uint32_t					GetDispatchedPacketCount()	const { return mDispatchedPacketCount; }

		const deque< InFlightPacket >&	GetInFlightPackets()	const { return mInFlightPackets; }
	private:



		InFlightPacket*		WriteSequenceNumber( OutputBitStream& inOutputStream,
			ClientProxy* inClientProxy );

		bool				ProcessSequenceNumber( InputBitStream& inInputStream );


		void				HandlePacketDeliveryFailure( const InFlightPacket& inFlightPacket );
		void				HandlePacketDeliverySuccess( const InFlightPacket& inFlightPacket );


		PacketSN	mNextOutgoingSequenceNumber;
		PacketSN	mNextExpectedSequenceNumber;

		deque< InFlightPacket >	mInFlightPackets;

		bool					mShouldSendAcks;
		bool					mShouldProcessAcks;

		uint32_t				mDeliveredPacketCount;
		uint32_t				mDroppedPacketCount;
		uint32_t				mDispatchedPacketCount;

	protected:
		AckBitField*			mAckBitField;
		void					ProcessAckBitField( InputBitStream& inInputStream );
	};



	inline InFlightPacket* DeliveryNotifyMgr::WriteState( OutputBitStream& inOutputStream,
		ClientProxy* inClientProxy )
	{
		InFlightPacket* toRet = WriteSequenceNumber( inOutputStream, inClientProxy );
		if ( mShouldSendAcks )
		{
			mAckBitField->Write( inOutputStream );
		}
		return toRet;
	}

	inline bool DeliveryNotifyMgr::ReadAndProcessState( InputBitStream& inInputStream )
	{
		bool toRet = ProcessSequenceNumber( inInputStream );
		if ( mShouldProcessAcks )
		{
			ProcessAckBitField( inInputStream );
		}
		return toRet;
	}

}