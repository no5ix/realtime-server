#include "AckBitField.h"


void AckBitField::AddToAckBitField( PacketSN inSequenceNumber, PacketSN inLastSN )
{
	mLatestAckSN = inSequenceNumber;

	static bool isFirstTime = true;
	if ( isFirstTime )
	{
		isFirstTime = false;
		return;
	}

	PacketSN difference = 0;
	for ( ; RealTimeSrvHelper::SequenceGreaterThan( inSequenceNumber, inLastSN++ ); ++difference );

	R_LOG_N_EXTRA( "difference = ", difference );

	uint8_t temp_uint8 = 0;
	for ( int i = ACK_BIT_FIELD_BYTE_LEN - 1; i > 0; --i )
	{
		*( mAckBitField + i ) = ( *( mAckBitField + i ) << difference );

		if ( i - 1 >= 0 )
		{
			temp_uint8 = *( mAckBitField + i - 1 );

			*( mAckBitField + i ) |=
				( temp_uint8 >> ( 8 - difference ) );
		}
	}

	uint8_t tempMask = 0x01 << ( difference - 1 );
	( *mAckBitField ) =
		tempMask |
		( ( *mAckBitField ) << difference );
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
