// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "InFlightPacket.h"
#include "RealTimeSrvTiming.h"



InFlightPacket::InFlightPacket( PacketSequenceNumber inSequenceNumber ) :
	mSequenceNumber( inSequenceNumber ),
	mTimeDispatched( RealTimeSrvTiming::sInstance.GetCurrentGameTime() )
{
	//null out other transmision data params...
}


void InFlightPacket::HandleDeliveryFailure( DeliveryNotificationMgr* inDeliveryNotificationManager ) const
{
	for ( const auto& pair : mTransmissionDataMap )
	{
		pair.second->HandleDeliveryFailure( inDeliveryNotificationManager );
	}
}

void InFlightPacket::HandleDeliverySuccess( DeliveryNotificationMgr* inDeliveryNotificationManager ) const
{
	for ( const auto& pair : mTransmissionDataMap )
	{
		pair.second->HandleDeliverySuccess( inDeliveryNotificationManager );
	}
}