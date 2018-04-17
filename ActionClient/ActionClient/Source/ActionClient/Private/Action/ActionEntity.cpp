// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionEntity.h"


void AActionEntity::Read( InputMemoryBitStream& inInputStream )
{

	bool stateBit;

	uint32_t readState = 0;

	inInputStream.Read( stateBit );
	if (stateBit)
	{
		uint32_t playerId;
		inInputStream.Read( playerId );
		SetPlayerId( playerId );
		readState |= ECRS_PlayerId;
	}

	//float oldRotation = GetRotation();
	//Vector3 oldLocation = GetLocation();
	//Vector3 oldVelocity = GetActionEntityVelocity();

	float replicatedRotation;
	Vector3 replicatedLocation;
	Vector3 replicatedVelocity;

	inInputStream.Read( stateBit );
	if (stateBit)
	{
		inInputStream.Read( replicatedVelocity.mX );
		inInputStream.Read( replicatedVelocity.mY );

		SetActionEntityVelocity( replicatedVelocity );

		inInputStream.Read( replicatedLocation.mX );
		inInputStream.Read( replicatedLocation.mY );

		SetLocation( replicatedLocation );

		inInputStream.Read( replicatedRotation );
		SetRotation( replicatedRotation );

		readState |= ECRS_Pose;
	}

	inInputStream.Read( stateBit );
	if (stateBit)
	{
		inInputStream.Read( stateBit );
		mThrustDir = stateBit ? 1.f : -1.f;
	}
	else
	{
		mThrustDir = 0.f;
	}

	inInputStream.Read( stateBit );
	if (stateBit)
	{
		Vector3 color;
		inInputStream.Read( color );
		SetColor( color );
		readState |= ECRS_Color;
	}

	inInputStream.Read( stateBit );
	if (stateBit)
	{
		mHealth = 0;
		inInputStream.Read( mHealth, 4 );
		readState |= ECRS_Health;
	}

	//if (GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
	//{
	//	//did we get health? if so, tell the hud!
	//	if (( readState & ECRS_Health ) != 0)
	//	{
	//		HUD::sInstance->SetPlayerHealth( mHealth );
	//	}
	//}
}