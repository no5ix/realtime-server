
#include "realtime_srv/math/Vector3.h"
#include "realtime_srv/math/Quaternion.h"

#include "realtime_srv/rep/BitStream.h"



using namespace realtime_srv;

void OutputBitStream::SliceTo(OutputBitStream& outputStream,
	uint8_t data, uint32_t bitCnt)
{
	uint32_t byteOffset = outputStream.bitHead_ >> 3;
	uint32_t bitOffset = outputStream.bitHead_ & 0x7;

	uint8_t currentMask = ~(0xff << bitOffset);

	outputStream.buffer_[byteOffset] =
		(outputStream.buffer_[byteOffset] & currentMask) | (data << bitOffset);


	uint32_t bitsFreeThisByte = 8 - bitOffset;

	if (bitsFreeThisByte < bitCnt)
	{
		outputStream.buffer_[byteOffset + 1] = data >> bitsFreeThisByte;
	}

	this->slicePoint_ += bitCnt;
	outputStream.bitHead_ += bitCnt;
}

bool OutputBitStream::SliceTo(OutputBitStream& outputStream)
{
	char * srcByte = this->buffer_ + (slicePoint_ >> 3);

	bool outIsReachTheEnd = false;

	uint32_t refOutputBitStreamSurplusBitLen =
		outputStream.bitCapacity_ - outputStream.bitHead_;
	uint32_t sliceSize =
		bitHead_ - slicePoint_ <= refOutputBitStreamSurplusBitLen
		?
		(outIsReachTheEnd = true, bitHead_ - slicePoint_)
		:
		refOutputBitStreamSurplusBitLen;

	while (sliceSize > 8)
	{
		SliceTo(outputStream, *srcByte, 8);
		++srcByte;
		sliceSize -= 8;
	}
	if (sliceSize > 0)
	{
		SliceTo(outputStream, *srcByte, sliceSize);
	}

	return outIsReachTheEnd;
}

void OutputBitStream::WriteBits(uint8_t data, uint32_t bitCnt)
{
	uint32_t nextBitHead = bitHead_ + static_cast<uint32_t>(bitCnt);

	if (nextBitHead > bitCapacity_)
	{
		ReallocBuffer(std::max(bitCapacity_ * 2, nextBitHead));
	}

	uint32_t byteOffset = bitHead_ >> 3;
	uint32_t bitOffset = bitHead_ & 0x7;

	uint8_t currentMask = ~(0xff << bitOffset);
	buffer_[byteOffset] = (buffer_[byteOffset] & currentMask) | (data << bitOffset);

	uint32_t bitsFreeThisByte = 8 - bitOffset;

	if (bitsFreeThisByte < bitCnt)
	{
		buffer_[byteOffset + 1] = data >> bitsFreeThisByte;
	}

	bitHead_ = nextBitHead;
}

void OutputBitStream::WriteBits(const void* data, uint32_t bitCnt)
{
	const char* srcByte = static_cast<const char*>(data);
	while (bitCnt > 8)
	{
		WriteBits(*srcByte, 8);
		++srcByte;
		bitCnt -= 8;
	}
	if (bitCnt > 0)
	{
		WriteBits(*srcByte, bitCnt);
	}
}

void OutputBitStream::Write(const Vector3& vec)
{
	Write(vec.X);
	Write(vec.Y);
	Write(vec.Z);
}

void InputBitStream::Read(Vector3& outVector)
{
	Read(outVector.X);
	Read(outVector.Y);
	Read(outVector.Z);
}

void OutputBitStream::Write(const Quaternion& quat)
{
	float precision = (2.f / 65535.f);
	Write(ConvertToFixed(static_cast<float> (quat.X), -1.f, precision), 16);
	Write(ConvertToFixed(static_cast<float> (quat.Y), -1.f, precision), 16);
	Write(ConvertToFixed(static_cast<float> (quat.Z), -1.f, precision), 16);
	Write(quat.W < 0);
}

void OutputBitStream::ReallocBuffer(uint32_t newBitLength)
{
	if (buffer_ == nullptr)
	{
		buffer_ = static_cast<char*>(std::malloc(newBitLength >> 3));
		memset(buffer_, 0, newBitLength >> 3);
	}
	else
	{
		char* tempBuffer = static_cast<char*>(std::malloc(newBitLength >> 3));
		memset(tempBuffer, 0, newBitLength >> 3);
		memcpy(tempBuffer, buffer_, bitCapacity_ >> 3);
		std::free(buffer_);
		buffer_ = tempBuffer;
	}
	bitCapacity_ = newBitLength;
}

void InputBitStream::ReadBits(uint8_t& outData, uint32_t bitCnt)
{
	uint32_t byteOffset = bitHead_ >> 3;
	uint32_t bitOffset = bitHead_ & 0x7;

	outData = static_cast<uint8_t>(buffer_[byteOffset]) >> bitOffset;

	uint32_t bitsFreeThisByte = 8 - bitOffset;
	if (bitsFreeThisByte < bitCnt)
	{
		outData |= static_cast<uint8_t>(buffer_[byteOffset + 1]) << bitsFreeThisByte;
	}

	outData &= (~(0x00ff << bitCnt));

	bitHead_ += bitCnt;
}

void InputBitStream::ReadBits(void* outData, uint32_t bitCnt)
{
	uint8_t* destByte = reinterpret_cast<uint8_t*>(outData);
	while (bitCnt > 8)
	{
		ReadBits(*destByte, 8);
		++destByte;
		bitCnt -= 8;
	}
	if (bitCnt > 0)
	{
		ReadBits(*destByte, bitCnt);
	}
}

void InputBitStream::Read(Quaternion& outQuat)
{
	float precision = (2.f / 65535.f);

	uint32_t f = 0;

	Read(f, 16);
	outQuat.X = ConvertFromFixed(f, -1.f, precision);
	Read(f, 16);
	outQuat.Y = ConvertFromFixed(f, -1.f, precision);
	Read(f, 16);
	outQuat.Z = ConvertFromFixed(f, -1.f, precision);

	outQuat.W = sqrtf(static_cast<float> (1.f -
		outQuat.X * outQuat.X +
		outQuat.Y * outQuat.Y +
		outQuat.Z * outQuat.Z));
	bool isNegative;
	Read(isNegative);

	if (isNegative)
	{
		outQuat.W *= -1;
	}
}


void InputBitStream::RecombineTo(InputBitStream& inputStream)
{
	char * destByte = inputStream.buffer_
		+ (inputStream.recombinePoint_ >> 3);

	uint32_t SurplusBitLen = bitCapacity_ - bitHead_;

	ReadBits(destByte, SurplusBitLen);

	inputStream.recombinePoint_ += SurplusBitLen;
}