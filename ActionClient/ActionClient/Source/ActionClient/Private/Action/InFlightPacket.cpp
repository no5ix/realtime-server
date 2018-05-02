// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "InFlightPacket.h"
#include "ActionTiming.h"



InFlightPacket::InFlightPacket( PacketSequenceNumber inSequenceNumber ) :
	mSequenceNumber( inSequenceNumber ),
	mTimeDispatched( ActionTiming::sInstance.GetCurrentGameTime() )
{
	//null out other transmision data params...
}


void InFlightPacket::HandleDeliveryFailure( DeliveryNotificationManager* inDeliveryNotificationManager ) const
{
	for ( const auto& pair : mTransmissionDataMap )
	{
		pair.second->HandleDeliveryFailure( inDeliveryNotificationManager );
	}
}

void InFlightPacket::HandleDeliverySuccess( DeliveryNotificationManager* inDeliveryNotificationManager ) const
{
	for ( const auto& pair : mTransmissionDataMap )
	{
		pair.second->HandleDeliverySuccess( inDeliveryNotificationManager );
	}
}