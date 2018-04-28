// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionPawn.h"
#include "InputManager.h"
#include "ActionHelper.h"
#include "ActionTiming.h"
#include "NetworkManager.h"
#include "ActionPlayerController.h"
#include "Kismet/KismetMathLibrary.h"



uint32_t AActionPawn::GetClassId() const
{
	return 'CHRT';
}

AActionPawn::AActionPawn( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{

	// Create root component
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>( this, TEXT( "RootComponent" ) );

	// Create camera component 
	ActionPawnCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>( this, TEXT( "ActionPawnCamera" ) );
	ActionPawnCamera->SetupAttachment( RootComponent );

	FTransform ActionPawnCameraTransform( FRotator::ZeroRotator, FVector( 0.f, 0.f, 70.f ) );
	ActionPawnCamera->SetRelativeTransform( ActionPawnCameraTransform );

	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "PawnMesh1P" ) );
	Mesh1P->SetupAttachment( ActionPawnCamera );
	Mesh1P->bOnlyOwnerSee = true;
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;

	FTransform Mesh1PTransform( FRotator( 0.f, -90.f, 0.f ), FVector( 0.f, 0.f, -150.f ) );
	Mesh1P->SetRelativeTransform( Mesh1PTransform );

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "PawnMesh3P" ) );
	Mesh3P->SetupAttachment( RootComponent );
	Mesh3P->bOnlyOwnerSee = false;
	Mesh3P->bOwnerNoSee = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->PrimaryComponentTick.TickGroup = TG_PrePhysics;

	FTransform Mesh3PTransform( FRotator( 0.f, -90.f, 0.f ), FVector( 0.f, 0.f, -86.f ) );
	Mesh3P->SetRelativeTransform( Mesh3PTransform );

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseTurnRate = 2.f;
	BaseLookUpRate = 2.f;

	MaxSpeed = 440;
	Acceleration = 1000.f;
	Deceleration = 2000.f;
	TurningBoost = 8.0f;

	mVelocity = FVector::ZeroVector;
	mActionPawnCameraRotation = FRotator::ZeroRotator;

	bTestUpdateForDisconnect = false;
	mLastReadStateTimestamp = -1.f;


	mTimeLocationBecameOutOfSync = 0.f;
	mTimeVelocityBecameOutOfSync = 0.f;
	mTimeRotationBecameOutOfSync = 0.f;
}

// Called when the game starts or when spawned
void AActionPawn::BeginPlay()
{
	Super::BeginPlay();

	mActionPawnCameraRotation = GetActorRotation();
}


// Called every frame
void AActionPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (bTestUpdateForDisconnect)
	{
		Update();
	}
	//Update();
}

// Called to bind functionality to input
void AActionPawn::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	check( PlayerInputComponent );
	PlayerInputComponent->BindAxis( "MoveForward", this, &AActionPawn::MoveForward );
	PlayerInputComponent->BindAxis( "MoveRight", this, &AActionPawn::MoveRight );
	PlayerInputComponent->BindAxis( "Turn", this, &AActionPawn::Turn );
	PlayerInputComponent->BindAxis( "LookUp", this, &AActionPawn::LookUp );
}



void AActionPawn::MoveForward( float Val )
{
	//if (Val != 0.f)
	//{
		//const FQuat Rotation = GetActorQuat();
		//const FVector Direction = FQuatRotationMatrix( Rotation ).GetScaledAxis( EAxis::X );
		//ActionAddMovementInput( Direction * Val );

		InputManager::sInstance->HandleInput( InputManager::EIA_MoveForward, Val );
		//InputManager::sInstance->HandleInput( InputManager::EIA_MoveForward, 1.f );

		A_LOG_N("Val = ", Val);
	//}
}

void AActionPawn::MoveRight( float Val )
{
	//if (Val != 0.f)
	//{
		//const FQuat Rotation = GetActorQuat();
		//const FVector Direction = FQuatRotationMatrix( Rotation ).GetScaledAxis( EAxis::Y );
		//ActionAddMovementInput( Direction * Val );

		InputManager::sInstance->HandleInput( InputManager::EIA_MoveRight, Val );
		//InputManager::sInstance->HandleInput( InputManager::EIA_MoveRight, 1.f );

		
		A_LOG_N("Val = ", Val);
	//}
}

void AActionPawn::Turn( float Val )
{



	//APawn::AddControllerYawInput( Val );
	//if (Val != 0)
	//{
		//FRotator newRot( GetActorRotation() );
		//newRot.Yaw += ( BaseTurnRate * Val );
		//SetActorRotation( newRot );

		InputManager::sInstance->HandleInput( InputManager::EIA_Turn, Val );
		//InputManager::sInstance->HandleInput( InputManager::EIA_Turn, 5.f );

		
		A_LOG_N("Val = ", Val);

	//}

}

void AActionPawn::LookUp( float Val )
{
	//AddControllerPitchInput( Val );

	//if (ActionPawnCamera && Val != 0)
	if ( ActionPawnCamera )
	{
		//FRotator newRot( GetActorRotation() );
		//ActionPawnCameraRotationLocal.Yaw = newRot.Yaw;
		//ActionPawnCameraRotationLocal.Roll = newRot.Roll;
		//ActionPawnCameraRotationLocal.Pitch = FMath::Clamp( ( ActionPawnCameraRotationLocal.Pitch + ( -1 * BaseLookUpRate * Val ) ), -89.f, 89.f );
		//ActionPawnCamera->SetWorldRotation( ActionPawnCameraRotationLocal );

		//FRotator newRot( ActionPawnCamera->GetComponentRotation() );
		//newRot.Pitch = FMath::Clamp( ( newRot.Pitch + ( -1 * BaseLookUpRate * Val ) ), -89.f, 89.f );
		//ActionPawnCamera->SetWorldRotation( newRot );

		InputManager::sInstance->HandleInput( InputManager::EIA_LookUp, Val );

		
		A_LOG_N("Val = ", Val);
	}
}

void AActionPawn::Update()
{
	if ( GetPlayerId() == NetworkManager::sInstance->GetPlayerId() || bTestUpdateForDisconnect )
	{
		const Action* pendingMove = InputManager::sInstance->GetAndClearPendingAction();

		if (pendingMove) //is it time to sample a new move...
		{
			float deltaTime = pendingMove->GetDeltaTime();

			ProcessInput( deltaTime, pendingMove->GetInputState() );


			SimulateMovement();

			//A_LOG_M( "Client Move Time: %3.4f deltaTime: %3.4f left rot at %3.4f", latestMove.GetTimestamp(), deltaTime, GetRotation() );
		}
	}
	else
	{
		//SimulateMovement( ActionTiming::sInstance.GetDeltaTime() );

		DR( ActionTiming::sInstance.GetDeltaTime() );

		SetActorLocation( mLocation );

		if ( GetVelocity() == FVector::ZeroVector )
		{
			mTimeLocationBecameOutOfSync = 0.f;
		}
	}
}

void AActionPawn::ProcessInput( float inDeltaTime, const ActionInputState& inInputState )
{
	FRotator newRot( GetActorRotation() );
	newRot.Yaw += ( BaseTurnRate * inInputState.GetDesiredTurnAmount() );
	//SetActorRotation( newRot );
	SetRotation( newRot );

	mActionPawnCameraRotation.Yaw = newRot.Yaw;
	mActionPawnCameraRotation.Roll = newRot.Roll;
	mActionPawnCameraRotation.Pitch = FMath::Clamp( ( mActionPawnCameraRotation.Pitch + ( -1 * BaseLookUpRate * inInputState.GetDesiredLookUpAmount() ) ), -89.f, 89.f );
	//ActionPawnCamera->SetWorldRotation( ActionPawnCameraRotation );
	SetActionPawnCameraRotation( mActionPawnCameraRotation );

	ActionAddMovementInput( mActionPawnCameraRotation.Quaternion().GetForwardVector(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( mActionPawnCameraRotation.Quaternion().GetRightVector(), inInputState.GetDesiredMoveRightAmount() );


	ApplyControlInputToVelocity( inDeltaTime );

	FVector Delta = mVelocity * inDeltaTime;

	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		SetLocation( GetActorLocation() + Delta );
	}
}

void AActionPawn::SimulateMovement()
{
	SetVelocity();

	SetActorRotation( mRotation );

	ActionPawnCamera->SetWorldRotation( mActionPawnCameraRotation );


	SetActorLocation( mLocation );

}

void AActionPawn::DR(float inDeltaTime)
{
	FVector Delta = Velocity * inDeltaTime;
	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		SetLocation( GetActorLocation() + Delta );
	}
}


bool AActionPawn::IsExceedingMaxSpeed( float inMaxSpeed ) const
{
	inMaxSpeed = FMath::Max( 0.f, inMaxSpeed );
	const float MaxSpeedSquared = FMath::Square( inMaxSpeed );

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return ( mVelocity.SizeSquared() > MaxSpeedSquared * OverVelocityPercent );
}

void AActionPawn::ActionAddMovementInput( FVector WorldDirection, float ScaleValue /*= 1.0f*/ )
{
	ActionControlInputVector += WorldDirection * ScaleValue;
}

FVector AActionPawn::ActionConsumeMovementInputVector()
{
	ActionLastControlInputVector = ActionControlInputVector;
	ActionControlInputVector = FVector::ZeroVector;
	return ActionLastControlInputVector;
}

FVector AActionPawn::ActionGetPendingInputVector() const
{
	// There's really no point redirecting to the MovementComponent since GetInputVector is not virtual there, and it just comes back to us.
	return ActionControlInputVector;
}


void AActionPawn::ApplyControlInputToVelocity( float DeltaTime )
{
	const FVector ControlAcceleration = ActionGetPendingInputVector().GetClampedToMaxSize( 1.f );

	const float AnalogInputModifier = ( ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f );
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed( MaxPawnSpeed );

	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (mVelocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
			mVelocity = mVelocity + ( ControlAcceleration * mVelocity.Size() - mVelocity ) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (mVelocity.SizeSquared() > 0.f)
		{
			const FVector OldVelocity = mVelocity;
			const float VelSize = FMath::Max( mVelocity.Size() - FMath::Abs( Deceleration ) * DeltaTime, 0.f );
			mVelocity = mVelocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if (bExceedingMaxSpeed && mVelocity.SizeSquared() < FMath::Square( MaxPawnSpeed ))
			{
				mVelocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = ( IsExceedingMaxSpeed( MaxPawnSpeed ) ) ? mVelocity.Size() : MaxPawnSpeed;
	mVelocity += ControlAcceleration * FMath::Abs( Acceleration ) * DeltaTime;
	mVelocity = mVelocity.GetClampedToMaxSize( NewMaxSpeed );

	mVelocity.Z = 0.f;

	ActionConsumeMovementInputVector();
}

bool AActionPawn::IsFirstPerson() const
{
	return true;
	//return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

void AActionPawn::OnCameraUpdate( const FVector& CameraLocation, const FRotator& CameraRotation )
{
	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>( GetClass()->GetDefaultSubobjectByName( TEXT( "PawnMesh1P" ) ) );
	const FMatrix DefMeshLS = FRotationTranslationMatrix( DefMesh1P->RelativeRotation, DefMesh1P->RelativeLocation );
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch( CameraRotation.Pitch, 0.0f, 0.0f );
	const FRotator RotCameraYaw( 0.0f, CameraRotation.Yaw, 0.0f );

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix( RotCameraYaw, CameraLocation ) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix( RotCameraPitch ) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	Mesh1P->SetRelativeLocationAndRotation( PitchedMesh.GetOrigin(), PitchedMesh.Rotator() );
}


void AActionPawn::Read( InputMemoryBitStream& inInputStream )
{
	bool stateBit;

	uint32_t readState = 0;

	inInputStream.Read( stateBit );
	if ( stateBit )
	{
		uint32_t playerId;
		inInputStream.Read( playerId );
		SetPlayerId( playerId );
		readState |= ECRS_PlayerId;



		if ( GetPlayerId() == NetworkManager::sInstance->GetPlayerId() )
		{
			AActionPlayerController* const FirstPC = Cast<AActionPlayerController>( UGameplayStatics::GetPlayerController( GetWorld(), 0 ) );
			if ( FirstPC != nullptr && !( FirstPC->GetPawn() ) )
			{
				FirstPC->Possess( this );
			}
		}
	}

	FRotator oldRotation = GetActorRotation();
	FVector oldLocation = GetActorLocation();
	FVector oldVelocity = GetVelocity();
	FRotator oldActionPawnCameraRotation = GetActionPawnCameraRotation();

	//FRotator replicatedRotation;
	//FVector replicatedLocation;
	//FVector replicatedVelocity;
	//FVector replicatedActionPawnCameraRotation;

	inInputStream.Read( stateBit );
	if ( stateBit )
	{
		inInputStream.Read( mVelocity.X );
		inInputStream.Read( mVelocity.Y );
		inInputStream.Read( mVelocity.Z );

		//SetActionEntityVelocity( replicatedVelocity );

		inInputStream.Read( mLocation.X );
		inInputStream.Read( mLocation.Y );
		inInputStream.Read( mLocation.Z );

		//SetLocation( replicatedLocation );

		inInputStream.Read( mRotation.Pitch );
		inInputStream.Read( mRotation.Yaw );
		inInputStream.Read( mRotation.Roll );

		//SetRotation( replicatedRotation );

		inInputStream.Read( mActionPawnCameraRotation.Pitch );
		inInputStream.Read( mActionPawnCameraRotation.Yaw );
		inInputStream.Read( mActionPawnCameraRotation.Roll );

		//SetActionPawnCameraRotation(mActionPawnCameraRotation)

		readState |= ECRS_Pose;
	}

	if ( GetPlayerId() == NetworkManager::sInstance->GetPlayerId() )
	{

		ReplayForLocalPawn( readState );

		if ( ( readState & ECRS_PlayerId ) == 0 )
		{
			InterpolateClientSidePrediction( oldRotation, oldActionPawnCameraRotation, oldLocation, oldVelocity , false );
		}
	}
	else
	{
		// temporary for now, may use entity interp instead later.
		ReplayForRemotePawn( readState );

		if ( ( readState & ECRS_PlayerId ) == 0 )
		{
			InterpolateClientSidePrediction( oldRotation, oldActionPawnCameraRotation, oldLocation, oldVelocity , true );
		}

	}

	SimulateMovement();
}


void AActionPawn::ReplayForLocalPawn( uint32_t inReadState )
{
	if ( ( inReadState & ECRS_Pose ) != 0 )
	{
		const ActionList& moveList = InputManager::sInstance->GetActionList();

		for ( const Action& move : moveList )
		{
			float deltaTime = move.GetDeltaTime();
			ProcessInput( deltaTime, move.GetInputState() );

			//SimulateMovement( deltaTime );
		}
	}
}


void AActionPawn::ReplayForRemotePawn( uint32_t inReadState )
{
	if ( ( inReadState & ECRS_Pose ) != 0 )
	{

		float rtt = NetworkManager::sInstance->GetRoundTripTime();

		float deltaTime = 1.f / 30.f;

		while ( true )
		{
			if ( rtt < deltaTime )
			{
				DR( rtt );
				break;
			}
			else
			{
				DR( deltaTime );
				rtt -= deltaTime;
			}
		}
	}

}

void AActionPawn::InterpolateClientSidePrediction( const FRotator& inOldRotation, const FRotator& inoldActionPawnCameraRotation, const FVector& inOldLocation, const FVector& inOldVelocity, bool inIsForRemotePawn )
{
	if ( inOldRotation != GetRotation() && !inIsForRemotePawn )
	{
		A_LOG_1( "ERROR! Move replay ended with incorrect rotation!");
	}

	float roundTripTime = NetworkManager::sInstance->GetRoundTripTime();
	float time = ActionTiming::sInstance.GetFrameStartTime();

	if ( inIsForRemotePawn && inOldRotation != GetRotation() )
	{

		if ( mTimeRotationBecameOutOfSync == 0.f )
		{
			mTimeRotationBecameOutOfSync = time;
		}

		float durationOutOfSync = time - mTimeRotationBecameOutOfSync;
		if ( durationOutOfSync < roundTripTime )
		{
			FRotator newRot = UKismetMathLibrary::RLerp(
				inOldRotation,
				GetRotation(),
				durationOutOfSync / roundTripTime,
				true
			);
			SetRotation( newRot );
			//SetActorRotation( newRot );

			FRotator newActionPawnCameraRotation = UKismetMathLibrary::RLerp(
				inoldActionPawnCameraRotation,
				GetActionPawnCameraRotation(),
				durationOutOfSync / roundTripTime,
				true
			);
			SetActionPawnCameraRotation( newActionPawnCameraRotation );
			//ActionPawnCamera->SetWorldRotation( newActionPawnCameraRotation );

		}
	}
	else
	{
		mTimeRotationBecameOutOfSync = 0.f;
	}


	if ( inOldLocation != GetLocation() ) 
	{

		//float time = ActionTiming::sInstance.GetFrameStartTime();
		if ( mTimeLocationBecameOutOfSync == 0.f )
		{
			mTimeLocationBecameOutOfSync = time;
		}

		float durationOutOfSync = time - mTimeLocationBecameOutOfSync;
		if ( durationOutOfSync < roundTripTime )
		{
			FVector newLoc = UKismetMathLibrary::VLerp(
				inOldLocation,
				GetLocation(),
				inIsForRemotePawn ? durationOutOfSync / roundTripTime : 0.1f
			);
			SetLocation( newLoc );
			//SetActorLocation( newLoc );
		}
	}
	else
	{
		mTimeLocationBecameOutOfSync = 0.f;
	}


	if ( inOldVelocity != GetActionEntityVelocity() )
	{
		A_LOG_1( "ERROR! Move replay ended with incorrect velocity!");

		//float time = ActionTiming::sInstance.GetFrameStartTime();
		if ( mTimeVelocityBecameOutOfSync == 0.f )
		{
			mTimeVelocityBecameOutOfSync = time;
		}

		float durationOutOfSync = time - mTimeVelocityBecameOutOfSync;
		if ( durationOutOfSync < roundTripTime )
		{

			FVector newVel = UKismetMathLibrary::VLerp(
				inOldVelocity,
				GetActionEntityVelocity(),
				inIsForRemotePawn ? durationOutOfSync / roundTripTime : 0.1f
			);
			SetActionEntityVelocity( newVel );
		}

	}
	else
	{
		mTimeVelocityBecameOutOfSync = 0.f;
	}




}


