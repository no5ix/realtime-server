// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <unordered_map>
#include "TransmissionData.h"
/**
 * 
 */

class DeliveryNotifyMgr;

typedef uint16_t	PacketSequenceNumber;
#define MAX_PACKET_SEQUENCE_NUMBER			(65535)
#define HALF_MAX_PACKET_SEQUENCE_NUMBER     (32768)


class InFlightPacket
{
public:

	InFlightPacket( PacketSequenceNumber inSequenceNumber );

	PacketSequenceNumber GetSequenceNumber() const { return mSequenceNumber; }
	float				 GetTimeDispatched() const { return mTimeDispatched; }

	void 				 SetTransmissionData( int inKey, TransmissionDataPtr	inTransmissionData )
	{
		mTransmissionDataMap[inKey] = inTransmissionData;
	}
	const TransmissionDataPtr GetTransmissionData( int inKey ) const
	{
		auto it = mTransmissionDataMap.find( inKey );
		return ( it != mTransmissionDataMap.end() ) ? it->second : nullptr;
	}

	void			HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const;
	void			HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const;

private:
	PacketSequenceNumber	mSequenceNumber;
	float			mTimeDispatched;

	std::unordered_map< int, TransmissionDataPtr >	mTransmissionDataMap;
};