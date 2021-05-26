#include <cassert>
#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/common/RealtimeSrvHelper.h"

#include "realtime_srv/rep/AckBitField.h"


using namespace realtime_srv;

void AckBitField::AddLastBit( uint32_t totalDifference )
{
	uint32_t byteOffset = ( totalDifference - 1 ) >> 3;
	uint32_t bitOffset = ( totalDifference - 1 ) & 0x7;
	uint8_t tempMask = 0x01 << bitOffset;
	*( ackBitField_ + byteOffset ) |= tempMask;
}

void AckBitField::DoAddToAckBitField( uint32_t difference )
{
	uint8_t temp_uint8 = 0;
	for ( int i = ACK_BIT_FIELD_BYTE_LEN - 1; i > 0; --i )
	{
		*( ackBitField_ + i ) = ( *( ackBitField_ + i ) << difference );
		if ( i - 1 >= 0 )
		{
			temp_uint8 = *( ackBitField_ + i - 1 );
			*( ackBitField_ + i ) |=
				( temp_uint8 >> ( 8 - difference ) );
		}
	}
	*ackBitField_ = ( *ackBitField_ ) << difference;
}

void AckBitField::AddToAckBitField( PacketSN sequenceNum, PacketSN lastSN )
{
	latestAckSN_ = sequenceNum;
	static bool isFirstTime = true;
	if ( isFirstTime )
	{
		isFirstTime = false;
		return;
	}
	uint32_t totalDifference = 0;
	for ( ; RealtimeSrvHelper::SNGreaterThan(
		sequenceNum, lastSN++ ); ++totalDifference );

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

void AckBitField::Write( OutputBitStream& outputStream )
{
	outputStream.Write( latestAckSN_ );
	outputStream.WriteBytes( ackBitField_, ACK_BIT_FIELD_BYTE_LEN );
}

void AckBitField::Read( InputBitStream& inputStream )
{
	inputStream.Read( latestAckSN_ );
	inputStream.ReadBytes( ackBitField_, ACK_BIT_FIELD_BYTE_LEN );
}

bool AckBitField::IsSetCorrespondingAckBit( PacketSN ackSN )
{
	uint32_t difference = 0;
	for ( ; RealtimeSrvHelper::SNGreaterThan(
		latestAckSN_, ackSN++ ); ++difference );

	assert( difference );
	uint32_t byteOffset = ( difference - 1 ) >> 3;
	uint32_t bitOffset = ( difference - 1 ) & 0x7;
	uint8_t tempMask = 0x01 << bitOffset;
	return ( tempMask & ( *( ackBitField_ + byteOffset ) ) ) ? true : false;
}