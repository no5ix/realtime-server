// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <deque>
#include "ActionClient.h"

/**
 *
 */
class StateQueue
{
public:

	struct StateData
	{
	public:
		StateData() {}
		StateData( 
			//int inNetworkId, 
			const FRotator& inRotation, const FVector& inVelocity, const FVector& inLocation, const FRotator& inCameraRotation ) :
			//mNetworkId( inNetworkId ),
			mRotation( inRotation ),
			mVelocity( inVelocity ),
			mLocation( inLocation ),
			mCameraRotation( inCameraRotation )
		{}

		//const int GetNetworkId() const { return mNetworkId; }
		const FRotator & GetRotation() const { return mRotation; }
		const FVector & GetVelocity() const { return mVelocity; }
		const FVector & GetLocation() const { return mLocation; }
		const FRotator & GetCameraRotation() const { return mCameraRotation; }
	private:
		//int mNetworkId;
		FRotator mRotation;
		FVector mVelocity;
		FVector mLocation;
		FRotator mCameraRotation;
	};

	StateQueue();
	~StateQueue();
	void AddStateData( const FRotator& inRotation, const FVector& inVelocity, const FVector& inLocation, const FRotator& inCameraRotation  );
	bool GetStateData(StateData& outStateData);

private:
	std::deque<StateData> mStateQueue;
};
