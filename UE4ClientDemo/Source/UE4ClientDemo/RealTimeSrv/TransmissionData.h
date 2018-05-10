// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <memory>

class DeliveryNotifyMgr;

class TransmissionData
{
public:
	virtual void HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const = 0;
	virtual void HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const = 0;
};
typedef std::shared_ptr< TransmissionData > TransmissionDataPtr;