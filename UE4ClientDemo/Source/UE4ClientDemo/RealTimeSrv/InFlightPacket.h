// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <unordered_map>
#include "TransmissionData.h"
#include "RealTimeSrvHelper.h"
/**
 * 
 */

class DeliveryNotifyMgr;

class InFlightPacket
{
public:

	InFlightPacket( PacketSN inSequenceNumber );

	PacketSN GetSequenceNumber() const { return mSequenceNumber; }
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
	PacketSN	mSequenceNumber;
	float			mTimeDispatched;

	std::unordered_map< int, TransmissionDataPtr >	mTransmissionDataMap;
};