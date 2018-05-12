#include "RealTimeSrvPCH.h"



void AckBitField::AddToAckBitField( PacketSequenceNumber inSequenceNumber, PacketSequenceNumber inLastSN )
{
	mLastAckSN = inSequenceNumber;
	static bool isFirstTime = true;
	if ( isFirstTime )
	{
		isFirstTime = false;
		return;
	}
	PacketSequenceNumber difference = 0;
	for ( ; RealTimeSrvHelper::SequenceGreaterThan( inSequenceNumber, inLastSN++ ); ++difference );

	uint8_t mask = 0x01 << ( difference - 1 );

	*mAckBitField  =
		mask |
		( ( *mAckBitField ) << difference );
}

void AckBitField::Write( OutputBitStream& inOutputStream )
{
	inOutputStream.Write( mLastAckSN );
	inOutputStream.WriteBytes( mAckBitField, ACK_BIT_FIELD_BYTE_LEN );
}

void AckBitField::Read( InputBitStream& inInputStream )
{
	inInputStream.Read( mLastAckSN );
	inInputStream.ReadBytes( mAckBitField, ACK_BIT_FIELD_BYTE_LEN );
}
