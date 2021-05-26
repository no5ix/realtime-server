#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <cstring>
#include <memory>

#include "realtime_srv/common/RealtimeSrvMacro.h"


namespace realtime_srv
{

class Vector3;
class Quaternion;


inline uint32_t ConvertToFixed(float data, float min, float precision)
{
	return static_cast<int> ((data - min) / precision);
}

inline float ConvertFromFixed(uint32_t data, float min, float precision)
{
	return static_cast<float>(data) * precision + min;
}


class OutputBitStream
{
public:

	OutputBitStream() :
		buffer_(nullptr),
		slicePoint_(0),
		bitHead_(0)
	{
		ReallocBuffer(MAX_PACKET_BYTE_LENGTH * 8);
	}

	~OutputBitStream() { std::free(buffer_); }

	bool SliceTo(OutputBitStream& outputStream);
	void SliceTo(OutputBitStream& outputStream, uint8_t data, uint32_t bitCnt);

	void		WriteBits(uint8_t data, uint32_t bitCnt);
	void		WriteBits(const void* data, uint32_t bitCnt);

	const 	char*	GetBufferPtr()		const { return buffer_; }
	uint32_t		GetBitLength()		const { return bitHead_; }
	uint32_t		GetByteLength()		const { return (bitHead_ + 7) >> 3; }

	void WriteBytes(const void* data, uint32_t byteCnt) { WriteBits(data, byteCnt << 3); }

	/*
	void Write( uint32_t data, uint32_t bitCnt = 32 )	{ WriteBits( &data, bitCnt ); }
	void Write( int data, uint32_t bitCnt = 32 )		{ WriteBits( &data, bitCnt ); }
	void Write( float data )								{ WriteBits( &data, 32 ); }

	void Write( uint16_t data, uint32_t bitCnt = 16 )	{ WriteBits( &data, bitCnt ); }
	void Write( int16_t data, uint32_t bitCnt = 16 )	{ WriteBits( &data, bitCnt ); }

	void Write( uint8_t data, uint32_t bitCnt = 8 )	{ WriteBits( &data, bitCnt ); }
	*/

	template< typename T >
	void Write(T data, uint32_t bitCnt = sizeof(T) * 8)
	{
		static_assert(std::is_arithmetic< T >::value ||
			std::is_enum< T >::value,
			"Generic Write only supports primitive data types");
		WriteBits(&data, bitCnt);
	}

	void 		Write(bool data) { WriteBits(&data, 1); }

	void		Write(const Vector3& vec);
	void		Write(const Quaternion& quat);

	void Write(const std::string& str)
	{
		uint32_t elementCount = static_cast<uint32_t>(str.size());
		Write(elementCount);
		for (const auto& element : str)
		{
			Write(element);
		}
	}

	//void Write( const PacketSN& inPacketSN )
	//{
	//	inPacketSN.Write( *this );
	//}

	void		ReallocBuffer(uint32_t newBitCapacity);

private:

	char*		buffer_;
	uint32_t	slicePoint_;
	uint32_t	bitHead_;
	uint32_t	bitCapacity_;
};
typedef std::shared_ptr<OutputBitStream> OutputBitStreamPtr;


class InputBitStream
{
public:

	InputBitStream(const char* buffer = nullptr, const uint32_t bitCnt = 0) :
		bitHead_(0),
		bitCapacity_(bitCnt),
		recombinePoint_(0)
	{
		if (buffer)
		{
			int byteCount = bitCapacity_ / 8;
			buffer_ = static_cast<char*>(malloc(bitCnt));
			memcpy(buffer_, buffer, byteCount);
			isBufferOwner_ = true;
		}
		else
		{
			buffer_ = nullptr;
			isBufferOwner_ = false;
		}
	}

	InputBitStream(const InputBitStream& other) :
		bitHead_(other.bitHead_),
		bitCapacity_(other.bitCapacity_),
		recombinePoint_(other.recombinePoint_),
		isBufferOwner_(true)
	{
		int byteCount = bitCapacity_ / 8;
		buffer_ = static_cast<char*>(malloc(byteCount));
		memcpy(buffer_, other.buffer_, byteCount);
	}

	InputBitStream& operator=(const InputBitStream& other)
	{
		if (this == &other)
		{
			return *this;
		}

		bitCapacity_ = other.bitCapacity_;
		bitHead_ = other.bitHead_;
		recombinePoint_ = other.recombinePoint_;
		if (isBufferOwner_ && buffer_)
		{
			free(buffer_);
		}
		int byteCount = bitCapacity_ / 8;
		buffer_ = static_cast<char*>(malloc(byteCount));
		memcpy(buffer_, other.buffer_, byteCount);
		isBufferOwner_ = true;
		return *this;
	}

	~InputBitStream()
	{
		if (isBufferOwner_ && buffer_)
		{
			free(buffer_);
			buffer_ = nullptr;
		}
	}
	const 	char*	GetBufferPtr()		const { return buffer_; }
	uint32_t	GetRemainingBitCount() 	const { return bitCapacity_ - bitHead_; }

	void		ReadBits(uint8_t& outData, uint32_t bitCnt);
	void		ReadBits(void* outData, uint32_t bitCnt);

	void		ReadBytes(void* outData, uint32_t byteCnt) { ReadBits(outData, byteCnt << 3); }

	template< typename T >
	void Read(T& data, uint32_t bitCnt = sizeof(T) * 8)
	{
		static_assert(std::is_arithmetic< T >::value ||
			std::is_enum< T >::value,
			"Generic Read only supports primitive data types");
		ReadBits(&data, bitCnt);
	}

	void		Read(uint32_t& outData, uint32_t bitCnt = 32) { ReadBits(&outData, bitCnt); }
	void		Read(int& outData, uint32_t bitCnt = 32) { ReadBits(&outData, bitCnt); }
	void		Read(float& outData) { ReadBits(&outData, 32); }

	void		Read(uint16_t& outData, uint32_t bitCnt = 16) { ReadBits(&outData, bitCnt); }
	void		Read(int16_t& outData, uint32_t bitCnt = 16) { ReadBits(&outData, bitCnt); }

	void		Read(uint8_t& outData, uint32_t bitCnt = 8) { ReadBits(&outData, bitCnt); }
	void		Read(bool& outData) { ReadBits(&outData, 1); }

	void		Read(Quaternion& outQuat);

	void		ResetToCapacity(uint32_t inByteCapacity) { bitCapacity_ = inByteCapacity << 3; bitHead_ = 0; }
	void		ResetToCapacityFromBit(uint32_t inBitCapacity) { bitCapacity_ = inBitCapacity; bitHead_ = 0; }

	void		RecombineTo(InputBitStream& inputStream);

	void Reinit(uint32_t bitCnt)
	{
		if (isBufferOwner_ && buffer_)
		{
			free(buffer_);
		}
		int byteCount = bitCnt / 8;
		buffer_ = static_cast<char*>(malloc(byteCount));
		bitCapacity_ = bitCnt;
		bitHead_ = 0;
		recombinePoint_ = 0;
		isBufferOwner_ = true;
	}

	uint32_t	GetRecombinePoint() const { return recombinePoint_; }


	void Read(std::string& str)
	{
		uint32_t elementCount;
		Read(elementCount);
		str.resize(elementCount);
		for (auto& element : str)
		{
			Read(element);
		}
	}

	void Read(Vector3& vec);

	//void Read( PacketSN& inPacketSN )
	//{
	//	inPacketSN.Read( *this );
	//}

private:
	char*		buffer_;
	uint32_t	bitHead_;
	uint32_t	bitCapacity_;
	uint32_t	recombinePoint_;
	bool		isBufferOwner_;

};
typedef std::shared_ptr<InputBitStream> InputBitStreamPtr;

}