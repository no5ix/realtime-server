// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "BitStream.h"
#include <memory>
#include "RealTimeSrvEntity.generated.h"






UCLASS()
class ARealTimeSrvEntity : public APawn
{
	GENERATED_BODY()

public:
	// 'GOBJ' = 1196376650;
	virtual uint32_t GetClassId() const /*{ return 'GOBJ'; }*/ ;
	//virtual uint32_t GetClassId() const { return 1196376650; }

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
	virtual void	UpdateTargetState() {}

	virtual uint32_t	Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const { return 0; }
	virtual void		Read( InputBitStream& inInputStream ) {}


	void		SetPlayerId( uint32_t inPlayerId ) { mPlayerId = inPlayerId; }
	uint32_t	GetPlayerId()						const { return mPlayerId; }

	void			SetLocalVelocity( const FVector& inVelocity ) { mLocalVelocity = inVelocity; }
	const FVector&	GetLocalVelocity()						const { return mLocalVelocity; }

	const FVector&	GetServerVelocity()						const { return mVelocity; }


	UFUNCTION( BlueprintCallable, Category = "ActionServer" )
		virtual FVector GetVelocity() const override { return GetLocalVelocity(); }

	
	void SetVelocity(const FVector& inVelocity) { mVelocity = inVelocity; }

	void	SetIndexInWorld( int inIndex ) { mIndexInWorld = inIndex; }
	int		GetIndexInWorld()				const { return mIndexInWorld; }

	bool DoesWantToDie() const { return false; }

	virtual void	HandleDying() {}

	const FVector&		GetLocation()				const { return mLocation; }
	void		SetLocation( const FVector& inLocation ) { mLocation = inLocation; }

	void	SetRotation( const FRotator& inRotation ) { mRotation = inRotation; };
	FRotator	GetRotation()					const { return mRotation; }


	int			GetNetworkId()				const { return mNetworkId; }
	void		SetNetworkId( int inNetworkId ) { mNetworkId = inNetworkId;  }

protected:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
		FVector mLocalVelocity;


	FRotator mLocalRotation;

	FVector mLocalLocation;

protected:

	FVector											mVelocity;
	FVector											mLocation;
	FVector											mColor;

	FRotator										mRotation;

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

	//float				mThrustDir;
	int					mHealth;

	bool				mIsShooting;

	float mIsTimeToStartSimulateMovementForRemotePawn;
};

typedef ARealTimeSrvEntity*	RealTimeSrvEntityPtr;