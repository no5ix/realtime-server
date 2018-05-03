// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <memory>

class DeliveryNotificationMgr;

class TransmissionData
{
public:
	virtual void HandleDeliveryFailure( DeliveryNotificationMgr* inDeliveryNotificationManager ) const = 0;
	virtual void HandleDeliverySuccess( DeliveryNotificationMgr* inDeliveryNotificationManager ) const = 0;
};
typedef std::shared_ptr< TransmissionData > TransmissionDataPtr;