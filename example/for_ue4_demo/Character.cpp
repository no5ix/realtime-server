#include <realtime_srv/RealtimeServer.h>
#include "Character.h"

using namespace realtime_srv;

Character::Character() :
	GameObj(),
	BaseTurnRate( 2.f ),
	BaseLookUpRate( 2.f ),
	MaxSpeed( 440.f ),
	Acceleration( 1000.f ),
	Deceleration( 2000.f ),
	TurningBoost( 8.0f ),
	curCameraRotation_( Vector3::Zero() ),
	oldCameraRotation_( Vector3::Zero() ),
	oldrentVelocity_( Vector3::Zero() ),
	currentVelocity_( Vector3::Zero() ) {}


void Character::SetOldState() {
	GameObj::SetOldState();
	oldrentVelocity_ = currentVelocity_;
	oldCameraRotation_ = curCameraRotation_;
}

bool Character::IsStateDirty() {
	return GameObj::IsStateDirty()
		|| !RealtimeSrvMath::Is3DVectorEqual( oldrentVelocity_, currentVelocity_ )
		|| !RealtimeSrvMath::Is3DVectorEqual( oldCameraRotation_, curCameraRotation_ );
}

void Character::Update() {
	SetOldState();
	GameObj::Update();
	if ( IsStateDirty() ) {
		SetStateDirty( EPS_Pose );
	}
}

uint32_t Character::Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const {
	uint32_t writtenState = 0;

	if ( inDirtyState & EPS_PlayerId ) {
		inOutputStream.Write( ( bool )true );
		inOutputStream.Write( GetPlayerId() );

		writtenState |= EPS_PlayerId;
	} else {
		inOutputStream.Write( ( bool )false );
	}
	if ( inDirtyState & EPS_Pose ) {
		inOutputStream.Write( ( bool )true );

		inOutputStream.Write( GetLocation() );
		inOutputStream.Write( GetRotation() );
		inOutputStream.Write( GetVelocity() );
		inOutputStream.Write( GetCameraRotation() );

		writtenState |= EPS_Pose;
	} else {
		inOutputStream.Write( ( bool )false );
	}
	return writtenState;
}

void Character::ProcessInput( float inDeltaTime, const InputState& inInputState ) {
	currentRotation_ = inInputState.GetDesiredTurnRot();
	curCameraRotation_ = inInputState.GetDesiredLookUpRot();

	ActionAddMovementInput( curCameraRotation_.ToQuaternion() * Vector3::Forward(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( curCameraRotation_.ToQuaternion() * Vector3::Right(), inInputState.GetDesiredMoveRightAmount() );
	ApplyControlInputToVelocity( inDeltaTime );

	const Vector3& Delta = currentVelocity_ * inDeltaTime;
	if ( !Delta.IsNearlyZero( 1e-6f ) ) {
		SetLocation( GetLocation() + Delta );
	}
}

bool Character::IsExceedingMaxSpeed( float inMaxSpeed ) const {
	inMaxSpeed = RealtimeSrvMath::Max( 0.f, inMaxSpeed );
	const float MaxSpeedSquared = RealtimeSrvMath::Square( inMaxSpeed );

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return ( currentVelocity_.SizeSquared() > MaxSpeedSquared * OverVelocityPercent );
}

void Character::ActionAddMovementInput( const realtime_srv::Vector3& WorldDirection, float ScaleValue /*= 1.0f */ ) {
	ControlInputVector_ += WorldDirection * ScaleValue;
}

const Vector3& Character::ConsumeMovementInputVector() {
	LastControlInputVector_ = ControlInputVector_;
	ControlInputVector_ = Vector3::Zero();
	return LastControlInputVector_;
}

const Vector3& Character::GetPendingInputVector() const {
	// There's really no point redirecting to the MovementComponent since GetInputVector is not virtual there, and it just comes back to us.
	return ControlInputVector_;
}

void Character::ApplyControlInputToVelocity( float DeltaTime ) {
	const Vector3& ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize( 1.f );

	const float AnalogInputModifier = ( ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f );
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed( MaxPawnSpeed );

	if ( AnalogInputModifier > 0.f && !bExceedingMaxSpeed ) {
		// Apply change in velocity direction
		if ( currentVelocity_.SizeSquared() > 0.f ) {
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = RealtimeSrvMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
			currentVelocity_ = currentVelocity_ + ( ControlAcceleration * currentVelocity_.Size() - currentVelocity_ ) * TimeScale;
		}
	} else {
		// Dampen velocity magnitude based on deceleration.
		if ( currentVelocity_.SizeSquared() > 0.f ) {
			const Vector3& OldVelocity = currentVelocity_;
			const float VelSize = RealtimeSrvMath::Max( currentVelocity_.Size() - RealtimeSrvMath::Abs( Deceleration ) * DeltaTime, 0.f );
			currentVelocity_ = currentVelocity_.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if ( bExceedingMaxSpeed && currentVelocity_.SizeSquared() < RealtimeSrvMath::Square( MaxPawnSpeed ) ) {
				currentVelocity_ = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = ( IsExceedingMaxSpeed( MaxPawnSpeed ) ) ? currentVelocity_.Size() : MaxPawnSpeed;
	currentVelocity_ += ControlAcceleration * RealtimeSrvMath::Abs( Acceleration ) * DeltaTime;
	currentVelocity_ = currentVelocity_.GetClampedToMaxSize( NewMaxSpeed );

	currentVelocity_.Z = 0.f;

	ConsumeMovementInputVector();
}
