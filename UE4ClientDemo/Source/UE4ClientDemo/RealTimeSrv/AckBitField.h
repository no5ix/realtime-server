#pragma once

#include "RealTimeSrvHelper.h"
#include "BitStream.h"

class AckBitField
{
public:
	AckBitField() : mLastAckSN( 0 )
	{
		mAckBitField = ( char * )malloc( ACK_BIT_FIELD_BYTE_LEN );
		memset( mAckBitField, 0, ACK_BIT_FIELD_BYTE_LEN );
	}

	~AckBitField()
	{
		if ( mAckBitField )
		{
			free( mAckBitField );
		}
	}

	void					AddToAckBitField( PacketSequenceNumber inSequenceNumber, PacketSequenceNumber inLastSN );

	void					Write( OutputBitStream& inOutputStream );
	void					Read( InputBitStream& inInputStream );

	char *					GetAckBitField()	const { return mAckBitField; }
	PacketSequenceNumber 	GetLastAckSN()		const { return mLastAckSN; }

private:
	char* 					mAckBitField;
	PacketSequenceNumber	mLastAckSN;
};