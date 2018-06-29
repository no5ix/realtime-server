#include "realtime_srv/common/RealtimeSrvShared.h"

using namespace realtime_srv;

void AckBitField::AddLastBit( uint32_t inTotalDifference )
{
	uint32_t byteOffset = ( inTotalDifference - 1 ) >> 3;
	uint32_t bitOffset = ( inTotalDifference - 1 ) & 0x7;
	uint8_t tempMask = 0x01 << bitOffset;
	*( mAckBitField + byteOffset ) |= tempMask;
}

void AckBitField::DoAddToAckBitField( uint32_t inDifference )
{
	uint8_t temp_uint8 = 0;
	for ( int i = ACK_BIT_FIELD_BYTE_LEN - 1; i > 0; --i )
	{
		*( mAckBitField + i ) = ( *( mAckBitField + i ) << inDifference );
		if ( i - 1 >= 0 )
		{
			temp_uint8 = *( mAckBitField + i - 1 );
			*( mAckBitField + i ) |=
				( temp_uint8 >> ( 8 - inDifference ) );
		}
	}
	*mAckBitField = ( *mAckBitField ) << inDifference;
}

void AckBitField::AddToAckBitField( PacketSN inSequenceNumber, PacketSN inLastSN )
{
	mLatestAckSN = inSequenceNumber;
	static bool isFirstTime = true;
	if ( isFirstTime )
	{
		isFirstTime = false;
		return;
	}
	uint32_t totalDifference = 0;
	for ( ; RealtimeSrvHelper::SequenceGreaterThan(
		inSequenceNumber, inLastSN++ ); ++totalDifference );

	uint32_t tempDiff = totalDifference;
	while ( tempDiff > 8 )
	{
		DoAddToAckBitField( 8 );
		tempDiff -= 8;
	}
	if ( tempDiff > 0 )
	{
		DoAddToAckBitField( tempDiff );
	}
	AddLastBit( totalDifference );
}

void AckBitField::Write( OutputBitStream& inOutputStream )
{
	inOutputStream.Write( mLatestAckSN );
	inOutputStream.WriteBytes( mAckBitField, ACK_BIT_FIELD_BYTE_LEN );
}

void AckBitField::Read( InputBitStream& inInputStream )
{
	inInputStream.Read( mLatestAckSN );
	inInputStream.ReadBytes( mAckBitField, ACK_BIT_FIELD_BYTE_LEN );
}

bool AckBitField::IsSetCorrespondingAckBit( PacketSN inAckSN )
{
	uint32_t difference = 0;
	for ( ; RealtimeSrvHelper::SequenceGreaterThan(
		mLatestAckSN, inAckSN++ ); ++difference );

	assert( difference );
	uint32_t byteOffset = ( difference - 1 ) >> 3;
	uint32_t bitOffset = ( difference - 1 ) & 0x7;
	uint8_t tempMask = 0x01 << bitOffset;
	return ( tempMask & ( *( mAckBitField + byteOffset ) ) ) ? true : false;
}