// Fill out your copyright notice in the Description page of Project Settings.


#include "BitStream.h"


void OutputBitStream::WriteBits( uint8_t inData,
	uint32_t inBitCount )
{
	uint32_t nextBitHead = mBitHead + static_cast< uint32_t >( inBitCount );

	if ( nextBitHead > mBitCapacity )
	{
		ReallocBuffer( ( mBitCapacity * 2 ) > nextBitHead ? ( mBitCapacity * 2 ) : nextBitHead );
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


void test1()
{
	OutputBitStream mbs;

	mbs.WriteBits( 11, 5 );
	mbs.WriteBits( 52, 6 );
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

void InputBitStream::RecombineTo( InputBitStream& refInputBitStream )
{
	char * destByte = refInputBitStream.mBuffer + ( refInputBitStream.mRecombinePoint >> 3 );

	uint32_t SurplusBitLen = mBitCapacity - mBitHead;

	ReadBits( destByte, SurplusBitLen );

	refInputBitStream.mRecombinePoint += SurplusBitLen;
}
