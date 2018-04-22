// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "MemoryBitStream.h"
#include "ActionMath.h"
#include "ActionEntity.generated.h"


//#define CLASS_IDENTIFICATION( inCode, inClass ) \
//enum { kClassId = inCode }; \
//virtual uint32_t GetClassId() const { return kClassId; } \
//static AActionEntity* CreateInstance() { return static_cast< AActionEntity* >( new inClass() ); } \


UCLASS()
class AActionEntity : public APawn
{
	GENERATED_BODY()


public:
	//CLASS_IDENTIFICATION( 'AENT', GameObject )

	enum ECatReplicationState
	{
		ECRS_Pose = 1 << 0,
		ECRS_Color = 1 << 1,
		ECRS_PlayerId = 1 << 2,
		ECRS_Health = 1 << 3,

		ECRS_AllState = ECRS_Pose | ECRS_Color | ECRS_PlayerId | ECRS_Health
	};

public:
	virtual void	Update() {}
	void Read( InputMemoryBitStream& inInputStream );

	void		SetPlayerId( uint32_t inPlayerId ) { mPlayerId = inPlayerId; }
	uint32_t	GetPlayerId()						const { return mPlayerId; }

	void			SetActionEntityVelocity( const Vector3& inVelocity ) { mVelocity = inVelocity; }
	const Vector3&	GetActionEntityVelocity()						const { return mVelocity; }


	const Vector3&		GetLocation()				const { return mLocation; }
	void		SetLocation( const Vector3& inLocation ) { mLocation = inLocation; }

	void	SetRotation( float inRotation ) { mRotation = inRotation; };
	float	GetRotation()					const { return mRotation; }

	void		SetColor( const Vector3& inColor ) { mColor = inColor; }
	const Vector3&		GetColor()					const { return mColor; }

private:

	Vector3				mVelocity;
	Vector3											mLocation;
	Vector3											mColor;

	float											mRotation;
	float											mScale;
	int												mIndexInWorld;

	bool											mDoesWantToDie;

	int												mNetworkId;

	//float				mMaxLinearSpeed;
	//float				mMaxRotationSpeed;

	////bounce fraction when hitting various things
	//float				mWallRestitution;
	//float				mCatRestitution;


	uint32_t			mPlayerId;
protected:

	///move down here for padding reasons...

	float				mLastMoveTimestamp;

	float				mThrustDir;
	int					mHealth;

	bool				mIsShooting;
};
