#include "RealTimeSrvPCH.h"



void OutputBitStream::SliceTo( OutputBitStream& refOutputBitStream, uint8_t inData, uint32_t inBitCount  )
{
	uint32_t byteOffset = refOutputBitStream.mBitHead >> 3;
	uint32_t bitOffset = refOutputBitStream.mBitHead & 0x7;

	uint8_t currentMask = ~( 0xff << bitOffset );

	refOutputBitStream.mBuffer[byteOffset] = 
		( refOutputBitStream.mBuffer[byteOffset] & currentMask ) | ( inData << bitOffset );


	uint32_t bitsFreeThisByte = 8 - bitOffset;

	if ( bitsFreeThisByte < inBitCount )
	{
		refOutputBitStream.mBuffer[byteOffset + 1] = inData >> bitsFreeThisByte;
	}

	this->mSlicePoint += inBitCount;
	refOutputBitStream.mBitHead += inBitCount;
}

bool OutputBitStream::SliceTo( OutputBitStream& refOutputBitStream )
{
	char * srcByte = this->mBuffer + (mSlicePoint >> 3);

	bool outIsReachTheEnd = false;

	uint32_t refOutputBitStreamSurplusBitLen = refOutputBitStream.mBitCapacity - refOutputBitStream.mBitHead;
	uint32_t sliceSize =
		mBitHead - mSlicePoint <= refOutputBitStreamSurplusBitLen
		?
		( outIsReachTheEnd = true, mBitHead - mSlicePoint )
		:
		refOutputBitStreamSurplusBitLen;

	while ( sliceSize > 8 )
	{
		SliceTo( refOutputBitStream, *srcByte, 8 );
		++srcByte;
		sliceSize -= 8;
	}
	if ( sliceSize > 0 )
	{
		SliceTo( refOutputBitStream, *srcByte, sliceSize );
	}

	return outIsReachTheEnd;
}

void OutputBitStream::WriteBits( uint8_t inData, uint32_t inBitCount )
{
	uint32_t nextBitHead = mBitHead + static_cast< uint32_t >( inBitCount );

	if ( nextBitHead > mBitCapacity )
	{
		ReallocBuffer( std::max( mBitCapacity * 2, nextBitHead ) );
	}

	uint32_t byteOffset = mBitHead >> 3;
	uint32_t bitOffset = mBitHead & 0x7;

	uint8_t currentMask = ~( 0xff << bitOffset );
	mBuffer[byteOffset] = ( mBuffer[byteOffset] & currentMask ) | ( inData << bitOffset );

	uint32_t bitsFreeThisByte = 8 - bitOffset;

	if ( bitsFreeThisByte < inBitCount )
	{
		mBuffer[byteOffset + 1] = inData >> bitsFreeThisByte;
	}

	mBitHead = nextBitHead;
}

void OutputBitStream::WriteBits( const void* inData, uint32_t inBitCount )
{
	const char* srcByte = static_cast< const char* >( inData );
	while ( inBitCount > 8 )
	{
		WriteBits( *srcByte, 8 );
		++srcByte;
		inBitCount -= 8;
	}
	if ( inBitCount > 0 )
	{
		WriteBits( *srcByte, inBitCount );
	}
}

void OutputBitStream::Write( const Vector3& inVector )
{
	Write( inVector.X );
	Write( inVector.Y );
	Write( inVector.Z );
}

void InputBitStream::Read( Vector3& outVector )
{
	Read( outVector.X );
	Read( outVector.Y );
	Read( outVector.Z );
}

void OutputBitStream::Write( const Quaternion& inQuat )
{
	float precision = ( 2.f / 65535.f );
	Write( ConvertToFixed( static_cast< float > ( inQuat.X ), -1.f, precision ), 16 );
	Write( ConvertToFixed( static_cast< float > ( inQuat.Y ), -1.f, precision ), 16 );
	Write( ConvertToFixed( static_cast< float > ( inQuat.Z ), -1.f, precision ), 16 );
	Write( inQuat.W < 0 );
}

void OutputBitStream::ReallocBuffer( uint32_t inNewBitLength )
{
	if ( mBuffer == nullptr )
	{
		mBuffer = static_cast< char* >( std::malloc( inNewBitLength >> 3 ) );
		memset( mBuffer, 0, inNewBitLength >> 3 );
	}
	else
	{
		char* tempBuffer = static_cast< char* >( std::malloc( inNewBitLength >> 3 ) );
		memset( tempBuffer, 0, inNewBitLength >> 3 );
		memcpy( tempBuffer, mBuffer, mBitCapacity >> 3 );
		std::free( mBuffer );
		mBuffer = tempBuffer;
	}
	mBitCapacity = inNewBitLength;
}

void InputBitStream::ReadBits( uint8_t& outData, uint32_t inBitCount )
{
	uint32_t byteOffset = mBitHead >> 3;
	uint32_t bitOffset = mBitHead & 0x7;

	outData = static_cast< uint8_t >( mBuffer[byteOffset] ) >> bitOffset;

	uint32_t bitsFreeThisByte = 8 - bitOffset;
	if ( bitsFreeThisByte < inBitCount )
	{
		outData |= static_cast< uint8_t >( mBuffer[byteOffset + 1] ) << bitsFreeThisByte;
	}

	outData &= ( ~( 0x00ff << inBitCount ) );

	mBitHead += inBitCount;
}

void InputBitStream::ReadBits( void* outData, uint32_t inBitCount )
{
	uint8_t* destByte = reinterpret_cast< uint8_t* >( outData );
	while ( inBitCount > 8 )
	{
		ReadBits( *destByte, 8 );
		++destByte;
		inBitCount -= 8;
	}
	if ( inBitCount > 0 )
	{
		ReadBits( *destByte, inBitCount );
	}
}

void InputBitStream::Read( Quaternion& outQuat )
{
	float precision = ( 2.f / 65535.f );

	uint32_t f = 0;

	Read( f, 16 );
	outQuat.X = ConvertFromFixed( f, -1.f, precision );
	Read( f, 16 );
	outQuat.Y = ConvertFromFixed( f, -1.f, precision );
	Read( f, 16 );
	outQuat.Z = ConvertFromFixed( f, -1.f, precision );

	outQuat.W = sqrtf( static_cast< float > ( 1.f -
		outQuat.X * outQuat.X +
		outQuat.Y * outQuat.Y +
		outQuat.Z * outQuat.Z ) );
	bool isNegative;
	Read( isNegative );

	if ( isNegative )
	{
		outQuat.W *= -1;
	}
}


void InputBitStream::RecombineTo( InputBitStream& refInputBitStream )
{
	char * destByte = refInputBitStream.mBuffer + ( refInputBitStream.mRecombinePoint >> 3 );

	uint32_t SurplusBitLen = mBitCapacity - mBitHead;

	ReadBits( destByte, SurplusBitLen );

	refInputBitStream.mRecombinePoint += SurplusBitLen;
}