#include "RealTimeSrvPCH.h"

//zoom hardcoded at 100...if we want to lock players on screen, this could be calculated from zoom
const float HALF_WORLD_HEIGHT = 3.6f;
const float HALF_WORLD_WIDTH = 6.4f;

Character::Character() :
	Entity(),
	mPlayerId( 0 ),
	mIsShooting( false ),
	mHealth( 10 )
{
	//SetCollisionRadius( 0.5f );


	BaseTurnRate = 2.f;
	BaseLookUpRate = 2.f;

	MaxSpeed = 440.f;
	Acceleration = 1000.f;
	Deceleration = 2000.f;
	TurningBoost = 8.0f;

	Velocity = Vector3::Zero();
	ActionPawnCameraRotation = Vector3::Zero();
}

void Character::ProcessInput( float inDeltaTime, const InputState& inInputState )
{
	//process our input....

	//Vector3 newRot( GetRotation() );
	//mRotation.Y += ( BaseTurnRate * inInputState.GetDesiredTurnAmount() );
	mRotation = inInputState.GetDesiredTurnRot();
	//SetRotation( newRot );

	//ActionPawnCameraRotation.Y = mRotation.Y;
	//ActionPawnCameraRotation.Z = mRotation.Z;
	//ActionPawnCameraRotation.X = ActionServerMath::Clamp( 
	//	( ActionPawnCameraRotation.X + ( -1 * BaseLookUpRate * inInputState.GetDesiredLookUpAmount() ) ),
	//	-89.f, 
	//	89.f );
	ActionPawnCameraRotation = inInputState.GetDesiredLookUpRot();

	ActionAddMovementInput( ActionPawnCameraRotation.ToQuaternion() * Vector3::Forward(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( ActionPawnCameraRotation.ToQuaternion() * Vector3::Right(), inInputState.GetDesiredMoveRightAmount() );


	ApplyControlInputToVelocity( inDeltaTime );
}

void Character::SimulateMovement( float inDeltaTime )
{
	// Move actor
	Vector3 Delta = Velocity * inDeltaTime;

	if (!Delta.IsNearlyZero( 1e-6f ))
	{
		SetLocation( GetLocation() + Delta );
	}

}

bool Character::IsExceedingMaxSpeed( float inMaxSpeed ) const
{
	inMaxSpeed = RealTimeSrvMath::Max( 0.f, inMaxSpeed );
	const float MaxSpeedSquared = RealTimeSrvMath::Square( inMaxSpeed );

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return ( Velocity.SizeSquared() > MaxSpeedSquared * OverVelocityPercent );
}

void Character::ActionAddMovementInput( Vector3 WorldDirection, float ScaleValue /*= 1.0f*/ )
{
	ActionControlInputVector += WorldDirection * ScaleValue;
}

Vector3 Character::ActionConsumeMovementInputVector()
{
	ActionLastControlInputVector = ActionControlInputVector;
	ActionControlInputVector = Vector3::Zero();
	return ActionLastControlInputVector;
}

Vector3 Character::ActionGetPendingInputVector() const
{
	// There's really no point redirecting to the MovementComponent since GetInputVector is not virtual there, and it just comes back to us.
	return ActionControlInputVector;
}

void Character::ApplyControlInputToVelocity( float DeltaTime )
{
	const Vector3 ControlAcceleration = ActionGetPendingInputVector().GetClampedToMaxSize( 1.f );

	const float AnalogInputModifier = ( ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f );
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed( MaxPawnSpeed );

	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (Velocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = RealTimeSrvMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
			Velocity = Velocity + ( ControlAcceleration * Velocity.Size() - Velocity ) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (Velocity.SizeSquared() > 0.f)
		{
			const Vector3 OldVelocity = Velocity;
			const float VelSize = RealTimeSrvMath::Max( Velocity.Size() - RealTimeSrvMath::Abs( Deceleration ) * DeltaTime, 0.f );
			Velocity = Velocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if (bExceedingMaxSpeed && Velocity.SizeSquared() < RealTimeSrvMath::Square( MaxPawnSpeed ))
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = ( IsExceedingMaxSpeed( MaxPawnSpeed ) ) ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * RealTimeSrvMath::Abs( Acceleration ) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize( NewMaxSpeed );

	Velocity.Z = 0.f;

	ActionConsumeMovementInputVector();
}


void Character::Update()
{

}

uint32_t Character::Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const
{
	uint32_t writtenState = 0;

	if (inDirtyState & ECRS_PlayerId)
	{
		inOutputStream.Write( ( bool )true );
		inOutputStream.Write( GetPlayerId() );

		writtenState |= ECRS_PlayerId;
	}
	else
	{
		inOutputStream.Write( ( bool )false );
	}


	if (inDirtyState & ECRS_Pose)
	{
		inOutputStream.Write( ( bool )true );

		//Vector3 velocity = Velocity;
		inOutputStream.Write( Velocity.X );
		inOutputStream.Write( Velocity.Y );
		inOutputStream.Write( Velocity.Z );

		//Vector3 location = GetLocation();
		inOutputStream.Write( GetLocation().X );
		inOutputStream.Write( GetLocation().Y );
		inOutputStream.Write( GetLocation().Z );

		//Vector3 rotation = GetRotation();
		inOutputStream.Write( GetRotation().X );
		inOutputStream.Write( GetRotation().Y );
		inOutputStream.Write( GetRotation().Z );

		//Vector3 rotation = GetActionPawnCameraRotation();
		inOutputStream.Write( GetActionPawnCameraRotation().X );
		inOutputStream.Write( GetActionPawnCameraRotation().Y );
		inOutputStream.Write( GetActionPawnCameraRotation().Z );

		writtenState |= ECRS_Pose;
	}
	else
	{
		inOutputStream.Write( ( bool )false );
	}

	//LOG( " mPlayerId = $d Character::Write finished. writtenState = %d", mPlayerId, writtenState );

	return writtenState;


}


