#pragma once

#include <cstring>
#include <stdlib.h>
#include "realtime_srv/common/RealtimeSrvMacro.h"

namespace realtime_srv
{


class OutputBitStream;
class InputBitStream;

class AckBitField
{
public:
	AckBitField() : latestAckSN_( 0 )
	{
		ackBitField_ = ( char * )malloc( ACK_BIT_FIELD_BYTE_LEN );
		memset( ackBitField_, 0, ACK_BIT_FIELD_BYTE_LEN );
	}

	~AckBitField()
	{
		if ( ackBitField_ )
		{
			free( ackBitField_ );
		}
	}
	void					AddToAckBitField( PacketSN sequenceNum, PacketSN lastSN );
	bool					IsSetCorrespondingAckBit( PacketSN ackSN );

	void					Write( OutputBitStream& outputStream );
	void					Read( InputBitStream& inputStream );

	char *					GetAckBitField()		const { return ackBitField_; }
	PacketSN 				GetLatestAckSN()		const { return latestAckSN_; }
private:
	void					AddLastBit( uint32_t totalDifference );
	void					DoAddToAckBitField( uint32_t difference );

private:
	char* 					ackBitField_;
	PacketSN				latestAckSN_;
};

}