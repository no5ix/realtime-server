//#pragma once
//
//typedef unsigned short			PacketSequenceNumber;
//#define PACKET_SN_BIT_WIDE		( 16 )
//#define MAX_PACKET_SN			( 65535 )
//#define HALF_MAX_PACKET_SN		( 32768 )
//
//class InputBitStream;
//class OutputBitStream;
//
//struct PacketSN
//{
//	PacketSN( int inPacketSN = 0 ) : mPacketSN( inPacketSN ) {}
//
//	PacketSequenceNumber GetPacketSN() const { return ( PacketSequenceNumber )( mPacketSN ); }
//
//	bool operator>=( const PacketSN& inOther ) const
//	{
//		return ( ( mPacketSN >= inOther.mPacketSN ) && ( mPacketSN - inOther.mPacketSN <= HALF_MAX_PACKET_SN ) ) ||
//			( ( mPacketSN < inOther.mPacketSN ) && ( inOther.mPacketSN - mPacketSN > HALF_MAX_PACKET_SN ) );
//	}
//
//	bool operator>( const PacketSN& inOther ) const
//	{
//		return ( ( mPacketSN > inOther.mPacketSN ) && ( mPacketSN - inOther.mPacketSN <= HALF_MAX_PACKET_SN ) ) ||
//			( ( mPacketSN < inOther.mPacketSN ) && ( inOther.mPacketSN - mPacketSN > HALF_MAX_PACKET_SN ) );
//	}
//
//	bool operator<=( const PacketSN& inOther ) const
//	{
//		return inOther >= ( *this );
//	}
//
//	bool operator<( const PacketSN& inOther ) const
//	{
//		return inOther > ( *this );
//	}
//
//	bool operator==( const PacketSN& inOther ) const
//	{
//		return mPacketSN == inOther.mPacketSN;
//	}
//
//	PacketSequenceNumber operator++( int )
//	{
//		return ++mPacketSN;
//	}
//
//	PacketSequenceNumber operator++()
//	{
//		return mPacketSN++;
//	}
//
//	PacketSequenceNumber operator--( int )
//	{
//		return --mPacketSN;
//	}
//
//	PacketSequenceNumber operator--()
//	{
//		return mPacketSN--;
//	}
//
//	PacketSequenceNumber operator-( const PacketSequenceNumber inPacketSequenceNumber ) const
//	{
//		return mPacketSN - inPacketSequenceNumber;
//	}
//
//	PacketSequenceNumber operator-( const PacketSN inPacketSN ) const
//	{
//		return mPacketSN - inPacketSN.mPacketSN;
//	}
//
//	friend PacketSequenceNumber operator-( const PacketSequenceNumber inPacketSequenceNumber, const PacketSN& inPacketSN )
//	{
//		return inPacketSequenceNumber - inPacketSN.mPacketSN;
//	}
//
//	PacketSequenceNumber operator+( const PacketSequenceNumber inPacketSequenceNumber ) const
//	{
//		return mPacketSN + inPacketSequenceNumber;
//	}
//
//	PacketSequenceNumber operator+( const PacketSN inPacketSN ) const
//	{
//		return mPacketSN + inPacketSN.mPacketSN;
//	}
//
//	friend PacketSequenceNumber operator+( const PacketSequenceNumber inPacketSequenceNumber, const PacketSN& inPacketSN )
//	{
//		return inPacketSequenceNumber + inPacketSN.mPacketSN;
//	}
//
//	PacketSequenceNumber operator=( const PacketSequenceNumber inPacketSequenceNumber )
//	{
//		mPacketSN = inPacketSequenceNumber;
//		return mPacketSN;
//	}
//
//	PacketSequenceNumber operator<<( int inLeftMoveCount ) const
//	{
//		return mPacketSN << inLeftMoveCount;
//	}
//
//	PacketSequenceNumber operator >> ( int inLeftMoveCount ) const
//	{
//		return mPacketSN >> inLeftMoveCount;
//	}
//
//	//operator int() const
//	//{
//	//	return ( int )mPacketSN;
//	//}
//
//	void Read( InputBitStream& inInputBitStream );
//
//
//	void Write( OutputBitStream& inOutputBitStream ) const;
//
//
//private:
//	PacketSequenceNumber mPacketSN;
//};