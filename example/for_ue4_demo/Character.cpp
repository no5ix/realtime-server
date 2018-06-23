#include <realtime_srv/RealtimeServer.h>
#include "Character.h"



Character::Character() :
	GameObj(),
	mPlayerId( 0 ),
	mIsShooting( false ),
	mHealth( 10 ),
	BaseTurnRate( 2.f ),
	BaseLookUpRate( 2.f ),
	MaxSpeed( 440.f ),
	Acceleration( 1000.f ),
	Deceleration( 2000.f ),
	TurningBoost( 8.0f ),
	CameraRotation_( Vector3::Zero() ),
	Velocity( Vector3::Zero() ),
	mTimeOfNextShot( 0.f ),
	mTimeBetweenShots( 0.2f )
{
	//SetCollisionRadius( 0.5f );
}

void Character::Update()
{
	Vector3 oldLocation = GetLocation();
	Vector3 oldVelocity = GetVelocity();
	Vector3 oldRotation = GetRotation();

	ClientProxyPtr client = NetworkMgr::sInstance->GetClientProxy( GetPlayerId() );
	if ( client )
	{
		ActionList& moveList = client->GetUnprocessedMoveList();
		for ( const Action& unprocessedMove : moveList )
		{
			const InputState& currentState = unprocessedMove.GetInputState();
			float deltaTime = unprocessedMove.GetDeltaTime();
			ProcessInput( deltaTime, currentState );
			SimulateMovement( deltaTime );

			//LOG( " CharacterServer::SimulateMovement() GetPlayerId = %d", GetPlayerId() );
		}

		moveList.Clear();
	}

	//HandleShooting();
	if ( !RealtimeSrvMath::Is3DVectorEqual( oldLocation, GetLocation() ) ||
		!RealtimeSrvMath::Is3DVectorEqual( oldVelocity, GetVelocity() ) ||
		!RealtimeSrvMath::Is3DVectorEqual( oldRotation, GetRotation() )
		)
	{
		NetworkMgr::sInstance->SetStateDirty( GetNetworkId(), ECRS_Pose );
	}
}

uint32_t Character::Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const
{
	uint32_t writtenState = 0;

	if ( inDirtyState & ECRS_PlayerId )
	{
		inOutputStream.Write( ( bool )true );
		inOutputStream.Write( GetPlayerId() );

		writtenState |= ECRS_PlayerId;
	}
	else
	{
		inOutputStream.Write( ( bool )false );
	}

	if ( inDirtyState & ECRS_Pose )
	{
		inOutputStream.Write( ( bool )true );

		inOutputStream.Write( Velocity );
		inOutputStream.Write( GetLocation() );
		inOutputStream.Write( GetRotation() );
		inOutputStream.Write( GetCameraRotation() );

		writtenState |= ECRS_Pose;
	}
	else
	{
		inOutputStream.Write( ( bool )false );
	}

	return writtenState;
}

void Character::HandleDying()
{
	NetworkMgr::sInstance->UnregistGameObjAndRetNetID( this );
}

void Character::HandleShooting()
{}

void Character::TakeDamage( int inDamagingPlayerId )
{}

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
	CameraRotation_ = inInputState.GetDesiredLookUpRot();

	ActionAddMovementInput( CameraRotation_.ToQuaternion() * Vector3::Forward(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( CameraRotation_.ToQuaternion() * Vector3::Right(), inInputState.GetDesiredMoveRightAmount() );


	ApplyControlInputToVelocity( inDeltaTime );
}

void Character::SimulateMovement( float inDeltaTime )
{
	// Move actor
	Vector3 Delta = Velocity * inDeltaTime;

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

	if ( AnalogInputModifier > 0.f && !bExceedingMaxSpeed )
	{
		// Apply change in velocity direction
		if ( Velocity.SizeSquared() > 0.f )
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = RealtimeSrvMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
			Velocity = Velocity + ( ControlAcceleration * Velocity.Size() - Velocity ) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if ( Velocity.SizeSquared() > 0.f )
		{
			const Vector3 OldVelocity = Velocity;
			const float VelSize = RealtimeSrvMath::Max( Velocity.Size() - RealtimeSrvMath::Abs( Deceleration ) * DeltaTime, 0.f );
			Velocity = Velocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if ( bExceedingMaxSpeed && Velocity.SizeSquared() < RealtimeSrvMath::Square( MaxPawnSpeed ) )
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = ( IsExceedingMaxSpeed( MaxPawnSpeed ) ) ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * RealtimeSrvMath::Abs( Acceleration ) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize( NewMaxSpeed );

	Velocity.Z = 0.f;

	ActionConsumeMovementInputVector();
}
