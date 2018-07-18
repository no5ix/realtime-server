#include <realtime_srv/RealtimeServer.h>
#include "Character.h"
#include "ExampleInputState.h"

using namespace realtime_srv;

Character::Character() :
	BaseTurnRate( 2.f ),
	BaseLookUpRate( 2.f ),
	MaxSpeed( 440.f ),
	Acceleration( 1000.f ),
	Deceleration( 2000.f ),
	TurningBoost( 8.0f ),
	playerId_( 0 ),
	curCameraRotation_( Vector3::Zero() ),
	oldCameraRotation_( Vector3::Zero() ),
	oldVelocity_( Vector3::Zero() ),
	curVelocity_( Vector3::Zero() ),
	curLocation_(
		2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
		2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
		0.f ),
	curRotation_( 0.f,
		RealtimeSrvMath::GetRandomFloat() * 180.f,
		0.f ),
	oldLocation_( Vector3::Zero() ),
	oldRotation_( Vector3::Zero() )
{}

void Character::BeforeProcessInput()
{
	oldLocation_ = curLocation_;
	oldRotation_ = curRotation_;
	oldVelocity_ = curVelocity_;
	oldCameraRotation_ = curCameraRotation_;
}

void Character::AfterProcessInput()
{
	if ( !RealtimeSrvMath::Is3DVectorEqual( oldLocation_, curLocation_ )
		|| !RealtimeSrvMath::Is3DVectorEqual( oldRotation_, curRotation_ )
		|| !RealtimeSrvMath::Is3DVectorEqual( oldVelocity_, curVelocity_ )
		|| !RealtimeSrvMath::Is3DVectorEqual( oldCameraRotation_, curCameraRotation_ ) )
	{
		SetStateDirty( ERS_Pose );
	}
}

uint32_t Character::Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const
{
	uint32_t writtenState = 0;

	if ( inDirtyState & ERS_PlayerId )
	{
		inOutputStream.Write( ( bool )true );
		inOutputStream.Write( playerId_ );

		writtenState |= ERS_PlayerId;
	}
	else
	{
		inOutputStream.Write( ( bool )false );
	}
	if ( inDirtyState & ERS_Pose )
	{
		inOutputStream.Write( ( bool )true );

		inOutputStream.Write( GetLocation() );
		inOutputStream.Write( GetRotation() );
		inOutputStream.Write( GetVelocity() );
		inOutputStream.Write( GetCameraRotation() );

		writtenState |= ERS_Pose;
	}
	else
	{
		inOutputStream.Write( ( bool )false );
	}
	return writtenState;
}

void Character::ProcessInput( float inDeltaTime, const InputStatePtr& inInputState )
{
	{	// test -> ExampleInputState
		const ExampleInputStatePtr& justForTest =
			std::dynamic_pointer_cast< ExampleInputState >( inInputState );
		if ( playerId_ != 0 ) // robot's playerId is 0
			assert( justForTest );
	}

	curRotation_ = inInputState->GetDesiredTurnRot();
	curCameraRotation_ = inInputState->GetDesiredLookUpRot();

	AddActionInput( curCameraRotation_.ToQuaternion() * Vector3::Forward(),
		inInputState->GetDesiredMoveForwardAmount() );

	AddActionInput( curCameraRotation_.ToQuaternion() * Vector3::Right(),
		inInputState->GetDesiredMoveRightAmount() );

	ApplyControlInputToVelocity( inDeltaTime );

	const Vector3& Delta = curVelocity_ * inDeltaTime;
	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		SetLocation( GetLocation() + Delta );
	}
}

bool Character::IsExceedingMaxSpeed( float inMaxSpeed ) const
{
	inMaxSpeed = RealtimeSrvMath::Max( 0.f, inMaxSpeed );
	const float MaxSpeedSquared = RealtimeSrvMath::Square( inMaxSpeed );

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return ( curVelocity_.SizeSquared() > MaxSpeedSquared * OverVelocityPercent );
}

void Character::AddActionInput( const Vector3& WorldDirection,
	float ScaleValue /*= 1.0f */ )
{
	ControlInputVector_ += WorldDirection * ScaleValue;
}

const Vector3& Character::ConsumeMovementInputVector()
{
	LastControlInputVector_ = ControlInputVector_;
	ControlInputVector_ = Vector3::Zero();
	return LastControlInputVector_;
}

const Vector3& Character::GetPendingInputVector() const
{
	// There's really no point redirecting to the MovementComponent since GetInputVector is not virtual there, and it just comes back to us.
	return ControlInputVector_;
}

void Character::ApplyControlInputToVelocity( float DeltaTime )
{
	const Vector3& ControlAcceleration =
		GetPendingInputVector().GetClampedToMaxSize( 1.f );

	const float AnalogInputModifier = ( ControlAcceleration.SizeSquared() > 0.f ?
		ControlAcceleration.Size() : 0.f );
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed( MaxPawnSpeed );

	if ( AnalogInputModifier > 0.f && !bExceedingMaxSpeed )
	{
		// Apply change in velocity direction
		if ( curVelocity_.SizeSquared() > 0.f )
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = RealtimeSrvMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
			curVelocity_ = curVelocity_ + ( ControlAcceleration *
				curVelocity_.Size() - curVelocity_ ) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if ( curVelocity_.SizeSquared() > 0.f )
		{
			const Vector3& OldVelocity = curVelocity_;
			const float VelSize = RealtimeSrvMath::Max( curVelocity_.Size() -
				RealtimeSrvMath::Abs( Deceleration ) * DeltaTime, 0.f );
			curVelocity_ = curVelocity_.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if ( bExceedingMaxSpeed && curVelocity_.SizeSquared() <
				RealtimeSrvMath::Square( MaxPawnSpeed ) )
			{
				curVelocity_ = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = ( IsExceedingMaxSpeed( MaxPawnSpeed ) ) ?
		curVelocity_.Size() : MaxPawnSpeed;
	curVelocity_ += ControlAcceleration * RealtimeSrvMath::Abs(
		Acceleration ) * DeltaTime;
	curVelocity_ = curVelocity_.GetClampedToMaxSize( NewMaxSpeed );

	curVelocity_.Z = 0.f;

	ConsumeMovementInputVector();
}
