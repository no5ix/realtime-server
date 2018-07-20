#pragma once

#include <cstring>
#include "realtime_srv/common/RealtimeSrvMacro.h"

namespace realtime_srv
{


class OutputBitStream;
class InputBitStream;

class AckBitField
{
public:
	AckBitField() : mLatestAckSN( 0 )
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
	void					AddToAckBitField( PacketSN inSequenceNumber, PacketSN inLastSN );
	bool					IsSetCorrespondingAckBit( PacketSN inAckSN );

	void					Write( OutputBitStream& inOutputStream );
	void					Read( InputBitStream& inInputStream );

	char *					GetAckBitField()		const { return mAckBitField; }
	PacketSN 				GetLatestAckSN()		const { return mLatestAckSN; }
private:
	void					AddLastBit( uint32_t inTotalDifference );
	void					DoAddToAckBitField( uint32_t inDifference );

private:
	char* 					mAckBitField;
	PacketSN				mLatestAckSN;
};

}