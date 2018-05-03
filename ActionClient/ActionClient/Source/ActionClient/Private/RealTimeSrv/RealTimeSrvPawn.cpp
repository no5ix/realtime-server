// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "RealTimeSrvPawn.h"
#include "InputManager.h"
#include "RealTimeSrvHelper.h"
#include "RealTimeSrvTiming.h"
#include "NetworkMgr.h"
#include "Kismet/KismetMathLibrary.h"



uint32_t ARealTimeSrvPawn::GetClassId() const
{
	return 'CHRT';
}

ARealTimeSrvPawn::ARealTimeSrvPawn( const FObjectInitializer& ObjectInitializer )
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

	mCameraRotation = FRotator::ZeroRotator;
	mLocalActionPawnCameraRotation = FRotator::ZeroRotator;

	mLocalRotation = FRotator::ZeroRotator;

	mLocalLocation = FVector::ZeroVector;

	bIsPlayerLocationOutOfSync = false;

	bIsRemotePlayerRotationOutOfSync = false;
	bIsRemotePlayerCameraRotationOutOfSync = false;
	bIsRemotePlayerVelocityOutOfSync = false;

	mLocalVelocity = FVector::ZeroVector;

	bTestUpdateForDisconnect = false;
	mLastDragTimestamp = -1.f;


	mTimeLocationBecameOutOfSync = 0.f;
	mTimeVelocityBecameOutOfSync = 0.f;
	mTimeRotationBecameOutOfSync = 0.f;



	mRemotePawnTargetLocation = FVector::ZeroVector;
	mRemotePawnTargetVelocity = FVector::ZeroVector;
	mRemotePawnTargetRotation = FRotator::ZeroRotator;
	mRemotePawnTargetCameraRotation = FRotator::ZeroRotator;

	mInterpSpeedToRemotePawnTargetLocation = 0.f;
	mInterpSpeedToRemotePawnTargetVelocity = 0.f;
	mInterpSpeedToRemotePawnTargetRotation = 0.f;
	mInterpSpeedToRemotePawnTargetCameraRotation = 0.f;

	mTimeOfLastUpdateTargetState = 0.f;
}

void ARealTimeSrvPawn::BeginPlay()
{
	Super::BeginPlay();

	mCameraRotation = GetActorRotation();
	mLocalActionPawnCameraRotation = mCameraRotation;


	// for test start

	//float f = 0.00123f;
	//float x = 107.582947f;
	//float y = 426.617615f;
	//float z = 0.000000f;
	//int count = 10000;
	//while ( count-- )
	//{

	//	ActionControlInputVector = FVector( x, y, z );
	//	ApplyControlInputToVelocity( f );
	//	A_LOG_M( "( GetVelocity() ).X << %f,  << ( GetVelocity() ).Y << %f,  << ( GetVelocity() ).Z << %f",
	//		( GetActionEntityVelocity() ).X, ( GetActionEntityVelocity() ).Y, ( GetActionEntityVelocity() ).Z
	//	);
	//	y++;
	//	z++;
	//	x++;
	//}

	// for test end

}

void ARealTimeSrvPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if ( bTestUpdateForDisconnect )
	{
		Update();
	}
	//Update();
}

void ARealTimeSrvPawn::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	check( PlayerInputComponent );
	PlayerInputComponent->BindAxis( "MoveForward", this, &ARealTimeSrvPawn::MoveForward );
	PlayerInputComponent->BindAxis( "MoveRight", this, &ARealTimeSrvPawn::MoveRight );
	PlayerInputComponent->BindAxis( "Turn", this, &ARealTimeSrvPawn::Turn );
	PlayerInputComponent->BindAxis( "LookUp", this, &ARealTimeSrvPawn::LookUp );
}

void ARealTimeSrvPawn::MoveForward( float Val )
{
	//if (Val != 0.f)
	//{
		//const FQuat Rotation = GetActorQuat();
		//const FVector Direction = FQuatRotationMatrix( Rotation ).GetScaledAxis( EAxis::X );
		//ActionAddMovementInput( Direction * Val );

	InputManager::sInstance->HandleMoveInput( InputManager::EIA_MoveForward, Val );
	//InputManager::sInstance->HandleInput( InputManager::EIA_MoveForward, 1.f );

	//A_LOG_N_EXTRA("Val = ", Val);
//}
}

void ARealTimeSrvPawn::MoveRight( float Val )
{
	//if (Val != 0.f)
	//{
		//const FQuat Rotation = GetActorQuat();
		//const FVector Direction = FQuatRotationMatrix( Rotation ).GetScaledAxis( EAxis::Y );
		//ActionAddMovementInput( Direction * Val );

	InputManager::sInstance->HandleMoveInput( InputManager::EIA_MoveRight, Val );
	//InputManager::sInstance->HandleInput( InputManager::EIA_MoveRight, 1.f );


	//A_LOG_N("Val = ", Val);
//}
}

void ARealTimeSrvPawn::Turn( float Val )
{
	//APawn::AddControllerYawInput( Val );
	//if (Val != 0)
	//{
	mLocalRotation = GetActorRotation();
	mLocalRotation.Yaw += ( BaseTurnRate * Val );

	InputManager::sInstance->HandleTurnInput(
		InputManager::EIA_Turn,
		mLocalRotation.Pitch,
		mLocalRotation.Yaw,
		mLocalRotation.Roll
	);

	//InputManager::sInstance->HandleInput( InputManager::EIA_Turn, 5.f );
	//A_LOG_N( "Val = ", Val );
	//A_LOG_N( "mLocalRotation.Yaw = ", mLocalRotation.Yaw );
//}

}

void ARealTimeSrvPawn::LookUp( float Val )
{
	//AddControllerPitchInput( Val );

	//if (ActionPawnCamera && Val != 0)
	if ( ActionPawnCamera )
	{
		//FRotator newRot( GetActorRotation() );
		mLocalActionPawnCameraRotation.Yaw = GetActorRotation().Yaw;
		mLocalActionPawnCameraRotation.Roll = GetActorRotation().Roll;
		//mLocalActionPawnCameraRotation.Pitch = FMath::Clamp( ( mLocalActionPawnCameraRotation.Pitch + ( -1 * BaseLookUpRate * inInputState.GetDesiredLookUpAmount() ) ), -89.f, 89.f );
		//ActionPawnCamera->SetWorldRotation( mLocalActionPawnCameraRotation );

		//mLocalActionPawnCameraRotation = ActionPawnCamera->GetComponentRotation();
		mLocalActionPawnCameraRotation.Pitch = FMath::Clamp( ( mLocalActionPawnCameraRotation.Pitch + ( -1 * BaseLookUpRate * Val ) ), -89.f, 89.f );
		//ActionPawnCamera->SetWorldRotation( mLocalActionPawnCameraRotation );

		InputManager::sInstance->HandleTurnInput(
			InputManager::EIA_LookUp,
			mLocalActionPawnCameraRotation.Pitch,
			mLocalActionPawnCameraRotation.Yaw,
			mLocalActionPawnCameraRotation.Roll
		);

		//A_LOG_N("Val = ", Val);
	}
}

void ARealTimeSrvPawn::Update()
{
	if ( GetPlayerId() == NetworkMgr::sInstance->GetPlayerId() || bTestUpdateForDisconnect )
	{
		const Action* pendingMove = InputManager::sInstance->GetAndClearPendingAction();

		if ( pendingMove ) //is it time to sample a new move...
		{
			float deltaTime = pendingMove->GetDeltaTime();

			//ProcessInput( deltaTime, pendingMove->GetInputState() );

			//SimulateMovementAfterReplay();

			ProcessInputBaseOnLocalState( deltaTime, pendingMove->GetInputState() );
			SimulateMovementForLocalPawn( deltaTime );
		}
	}
	else
	{
		//DR( ActionTiming::sInstance.GetDeltaTime() );
		//SimulateMovementForRemotePawn( ActionTiming::sInstance.GetDeltaTime() );
		////SetActorLocation( mLocation );

		//if ( GetVelocity() == FVector::ZeroVector )
		//{
		//	mTimeLocationBecameOutOfSync = 0.f;
		//}

		SimulateMovementForRemotePawn( RealTimeSrvTiming::sInstance.GetDeltaTime() );
	}
}

void ARealTimeSrvPawn::ProcessInputBaseOnLocalState( float inDeltaTime, const RealTimeSrvInputState& inInputState )
{
	//FRotator newRot( GetActorRotation() );
	//newRot.Yaw += ( BaseTurnRate * inInputState.GetDesiredTurnAmount() );
	//SetActorRotation( mLocalRotation );
	//SetRotation( newRot );

	//mLocalActionPawnCameraRotation.Yaw = newRot.Yaw;
	//mLocalActionPawnCameraRotation.Roll = newRot.Roll;
	//mLocalActionPawnCameraRotation.Pitch = FMath::Clamp( ( mLocalActionPawnCameraRotation.Pitch + ( -1 * BaseLookUpRate * inInputState.GetDesiredLookUpAmount() ) ), -89.f, 89.f );
	//ActionPawnCamera->SetWorldRotation( mLocalActionPawnCameraRotation );
	//SetActionPawnCameraRotation( mActionPawnCameraRotation );

	ActionAddMovementInput( mLocalActionPawnCameraRotation.Quaternion().GetForwardVector(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( mLocalActionPawnCameraRotation.Quaternion().GetRightVector(), inInputState.GetDesiredMoveRightAmount() );

	ApplyControlInputToVelocity( inDeltaTime );

	//SetVelocity();

	FVector Delta = mVelocity * inDeltaTime;

	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		//SetActorLocation( GetActorLocation() + Delta );
		mLocalLocation = GetActorLocation() + Delta;
	}
}

void ARealTimeSrvPawn::ProcessInputBaseOnServerState( float inDeltaTime, const RealTimeSrvInputState& inInputState )
{
	//FRotator newRot( GetActorRotation() );
	//mRotation.Yaw += ( BaseTurnRate * inInputState.GetDesiredTurnAmount() );
	mRotation = inInputState.GetDesiredTurnRot();

	//mActionPawnCameraRotation.Yaw = mRotation.Yaw;
	//mActionPawnCameraRotation.Roll = mRotation.Roll;
	//mActionPawnCameraRotation.Pitch = FMath::Clamp( ( mActionPawnCameraRotation.Pitch + ( -1 * BaseLookUpRate * inInputState.GetDesiredLookUpAmount() ) ), -89.f, 89.f );
	//ActionPawnCamera->SetWorldRotation( ActionPawnCameraRotation );
	mCameraRotation = inInputState.GetDesiredLookUpRot();

	ActionAddMovementInput( mCameraRotation.Quaternion().GetForwardVector(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( mCameraRotation.Quaternion().GetRightVector(), inInputState.GetDesiredMoveRightAmount() );

	ApplyControlInputToVelocity( inDeltaTime );

	FVector Delta = mVelocity * inDeltaTime;

	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		mLocation += Delta;
	}
}

void ARealTimeSrvPawn::UpdateTargetState()
{
	StateQueue::StateData stateData;
	if ( !mStateBuffer.GetStateData( stateData ) )
	{
		return;
		A_LOG_1_EXTRA("UpdateTargetState_return_with no state data");
	}

	mRemotePawnTargetLocation = stateData.GetLocation();
	mRemotePawnTargetVelocity = stateData.GetVelocity();
	mRemotePawnTargetRotation = stateData.GetRotation();
	mRemotePawnTargetCameraRotation = stateData.GetCameraRotation();


	mInterpSpeedToRemotePawnTargetLocation = 
		( mRemotePawnTargetLocation - GetActorLocation() ).Size() 
		/ 
		NetworkMgr::kTimeBetweenStatePackets;

	mInterpSpeedToRemotePawnTargetVelocity = 
		( mRemotePawnTargetVelocity - GetLocalVelocity() ).Size()
		/ 
		NetworkMgr::kTimeBetweenStatePackets;

	//mInterpSpeedToRemotePawnTargetRotation = 
	//	( mRemotePawnTargetRotation - GetActorRotation() ). 
	//	/ 
	//	NetworkManager::kTimeBetweenStatePackets;

	//mInterpSpeedToRemotePawnTargetCameraRotation = 
	//	( mRemotePawnTargetCameraRotation - ActionPawnCamera->GetComponentRotation() ).Size() 
	//	/ 
	//	NetworkManager::kTimeBetweenStatePackets;
	A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetLocation = %f, %f, %f", mRemotePawnTargetLocation.X, mRemotePawnTargetLocation.Y, mRemotePawnTargetLocation.Z );
	A_LOG_M_EXTRA( "READ!!! mInterpSpeedToRemotePawnTargetLocation = %f", mInterpSpeedToRemotePawnTargetLocation );
	A_LOG_1_EXTRA( "------------------------" );

	A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetVelocity = %f, %f, %f", mRemotePawnTargetVelocity.X, mRemotePawnTargetVelocity.Y, mRemotePawnTargetVelocity.Z );
	A_LOG_M_EXTRA( "READ!!! mInterpSpeedToRemotePawnTargetVelocity = %f", mInterpSpeedToRemotePawnTargetVelocity );
	A_LOG_1_EXTRA( "------------------------" );

	A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetRotation = %f, %f, %f", mRemotePawnTargetRotation.Pitch, mRemotePawnTargetRotation.Yaw, mRemotePawnTargetRotation.Roll );
	A_LOG_1_EXTRA( "------------------------" );

	A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetCameraRotation = %f, %f, %f", mRemotePawnTargetCameraRotation.Pitch, mRemotePawnTargetCameraRotation.Yaw, mRemotePawnTargetCameraRotation.Roll );


}

void ARealTimeSrvPawn::SimulateMovementForRemotePawn( float inDeltaTime )
{
	{
		A_LOG_1_EXTRA( "~~%%%%%%%%%%%%%%%%%%%SimulateMovementForRemotePawn%%%%%%Before%%%%%%%%%%%%%~~~~" )
			A_LOG_M_EXTRA( "READ!!! GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
		A_LOG_M_EXTRA( "READ!!! GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );
		A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetLocation = %f, %f, %f", mRemotePawnTargetLocation.X, mRemotePawnTargetLocation.Y, mRemotePawnTargetLocation.Z );
		A_LOG_1_EXTRA( "------------------------" );
		A_LOG_M_EXTRA( "READ!!! mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );
		A_LOG_M_EXTRA( "READ!!! mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
		A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetVelocity = %f, %f, %f", mRemotePawnTargetVelocity.X, mRemotePawnTargetVelocity.Y, mRemotePawnTargetVelocity.Z );
		A_LOG_1_EXTRA( "------------------------" );
		A_LOG_M_EXTRA( "READ!!! GetActorRotation() = %f, %f, %f", GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll );
		A_LOG_M_EXTRA( "READ!!! GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
		A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetRotation = %f, %f, %f", mRemotePawnTargetRotation.Pitch, mRemotePawnTargetRotation.Yaw, mRemotePawnTargetRotation.Roll );
		A_LOG_1_EXTRA( "------------------------" );
		A_LOG_M_EXTRA( "READ!!! ActionPawnCamera->GetComponentRotation() = %f, %f, %f", ActionPawnCamera->GetComponentRotation().Pitch, ActionPawnCamera->GetComponentRotation().Yaw, ActionPawnCamera->GetComponentRotation().Roll );
		A_LOG_M_EXTRA( "READ!!! mActionPawnCameraRotation = %f, %f, %f", mCameraRotation.Pitch, mCameraRotation.Yaw, mCameraRotation.Roll );
		A_LOG_M_EXTRA( "READ!!! mRemotePawnTargetCameraRotation = %f, %f, %f", mRemotePawnTargetCameraRotation.Pitch, mRemotePawnTargetCameraRotation.Yaw, mRemotePawnTargetCameraRotation.Roll );
		A_LOG_1_EXTRA( "%%%%%%%%%%%%%%%%%%%~~~SimulateMovementForRemotePawn%%%%%%%Before%%%%%%%%%%%%~~~" );
	}

	//A_LOG_N_EXTRA( "ActionTiming::sInstance.GetCurrentGameTime() = ", ActionTiming::sInstance.GetCurrentGameTime() );

	if ( RealTimeSrvTiming::sInstance.GetCurrentGameTime() < mIsTimeToStartSimulateMovementForRemotePawn )
	{
		A_LOG_1_EXTRA( "ActionTiming::sInstance.GetCurrentGameTime() < mIsTimeToStartSimulateMovementForRemotePawn" );
		return;
	}

	float time = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	if ( time > mTimeOfLastUpdateTargetState + NetworkMgr::kTimeBetweenStatePackets )
	{
		mTimeOfLastUpdateTargetState = time;
		UpdateTargetState();
	}

	//mLocalLocation = mRemotePawnTargetLocation;
	//mLocalVelocity = mRemotePawnTargetVelocity;

	//A_LOG_N_EXTRA( "VInterpTo_Constant start ActionTiming::sInstance.GetCurrentGameTime() = ", ActionTiming::sInstance.GetCurrentGameTime() );
	mLocalLocation = UKismetMathLibrary::VInterpTo_Constant(
		GetActorLocation(),
		mRemotePawnTargetLocation,
		inDeltaTime,
		mRemotePawnTargetVelocity.Size()
	);
	//A_LOG_N_EXTRA( "VInterpTo_Constant end ActionTiming::sInstance.GetCurrentGameTime() = ", ActionTiming::sInstance.GetCurrentGameTime() );


	mLocalVelocity = UKismetMathLibrary::VInterpTo_Constant(
		GetLocalVelocity(),
		mRemotePawnTargetVelocity,
		inDeltaTime,
		Acceleration * 2.f
	);

	//FVector Delta = mLocalVelocity * inDeltaTime;
	//if ( !Delta.IsNearlyZero( 1e-6f ) )
	//{
	//	//mLocalLocation = ( GetActorLocation() + Delta );
	//	mLocalLocation = ( mRemotePawnTargetLocation + Delta );
	//}


	mLocalRotation = UKismetMathLibrary::RInterpTo(
		GetActorRotation(),
		mRemotePawnTargetRotation,
		inDeltaTime,
		BaseTurnRate * 8.f
	);
	

	mLocalActionPawnCameraRotation = UKismetMathLibrary::RInterpTo(
		ActionPawnCamera->GetComponentRotation(),
		mRemotePawnTargetCameraRotation,
		inDeltaTime,
		BaseLookUpRate * 8.f
	);

	{
		A_LOG_1_EXTRA( "~~***************SimulateMovementForRemotePawn*%After***************%~~~~" );
		A_LOG_M_EXTRA( "READ!!! mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );
		A_LOG_1_EXTRA( "------------------------" );
		A_LOG_M_EXTRA( "READ!!! mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );
		A_LOG_1_EXTRA( "------------------------" );
		A_LOG_M_EXTRA( "READ!!! mLocalRotation = %f, %f, %f", mLocalRotation.Pitch, mLocalRotation.Yaw, mLocalRotation.Roll );
		A_LOG_1_EXTRA( "------------------------" );
		A_LOG_M_EXTRA( "READ!!! mLocalActionPawnCameraRotation = %f, %f, %f", mLocalActionPawnCameraRotation.Pitch, mLocalActionPawnCameraRotation.Yaw, mLocalActionPawnCameraRotation.Roll );
		A_LOG_1_EXTRA( "------------------------" );
		A_LOG_1_EXTRA( "***************~~~SimulateMovementForRemotePawn**************%After**************%%%%~~~" );
	}

	SetActorLocation( mLocalLocation );
	SetLocalVelocity( mLocalVelocity );
	SetActorRotation( mLocalRotation );
	ActionPawnCamera->SetWorldRotation( mLocalActionPawnCameraRotation );
}

//void AActionPawn::SimulateMovementForRemotePawn( float inDeltaTime )
//{
//	A_LOG_1_EXTRA( "~~*%%%%%%%%%%%%%%SimulateMovementForRemotePawn%%%%%%Before%%%%%%%%%%%%%~~~~" );
//
//	A_LOG_M_EXTRA( "READ!!! GetActorRotation() = %f, %f, %f", GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll );
//	A_LOG_M_EXTRA( "READ!!! GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
//	A_LOG_1_EXTRA( "------------------------" );
//
//	A_LOG_M_EXTRA( "READ!!! ActionPawnCamera->GetComponentRotation() = %f, %f, %f", ActionPawnCamera->GetComponentRotation().Pitch, ActionPawnCamera->GetComponentRotation().Yaw, ActionPawnCamera->GetComponentRotation().Roll );
//	A_LOG_M_EXTRA( "READ!!! mActionPawnCameraRotation = %f, %f, %f", mActionPawnCameraRotation.Pitch, mActionPawnCameraRotation.Yaw, mActionPawnCameraRotation.Roll );
//	A_LOG_1_EXTRA( "------------------------" );
//
//	A_LOG_M_EXTRA( "READ!!! GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
//	A_LOG_M_EXTRA( "READ!!! GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );
//	A_LOG_1_EXTRA( "------------------------" );
//
//	A_LOG_M_EXTRA( "READ!!! mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );
//	A_LOG_M_EXTRA( "READ!!! mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
//
//	A_LOG_1_EXTRA( "%%%%%%%%%%%%%%%%%%%~~~SimulateMovementForRemotePawn%%%%%%%Before%%%%%%%%%%%%~~~" );
//
//	//SetActorRotation( mRotation );
//	//ActionPawnCamera->SetWorldRotation( mActionPawnCameraRotation );
//	//SetActorLocation( mLocation );
//	//SetLocalVelocity(mVelocity);
//
//	//if ( bIsRemotePlayerVelocityOutOfSync )
//	//{
//	//	mLocalVelocity = UKismetMathLibrary::VInterpTo( GetLocalVelocity(), mVelocity, inDeltaTime, 40.f );
//	//	if ( mLocalVelocity.Equals( mVelocity, 1.f ) )
//	//		//if ( mLocalLocation == mLocation )
//	//	{
//	//		bIsRemotePlayerVelocityOutOfSync = false;
//	//	}
//	//}
//
//	if ( bIsPlayerLocationOutOfSync )
//	{
//		//mLocalLocation = UKismetMathLibrary::VInterpTo( GetActorLocation(), mLocation, inDeltaTime, FMath::Max( mVelocity.Size() / 25.f, 1.f ) );
//		//if ( mLocalLocation.Equals( mLocation, 1.f ) )
//		//	//if ( mLocalLocation == mLocation )
//		//{
//		//	bIsPlayerLocationOutOfSync = false;
//		//}
//		mLocalLocation = mLocation;
//	}
//
//	if ( bIsRemotePlayerRotationOutOfSync )
//	{
//		mLocalRotation = UKismetMathLibrary::RInterpTo(
//			GetActorRotation(),
//			mRotation,
//			inDeltaTime,
//			BaseTurnRate * 8.f
//		);
//
//		if ( mLocalRotation.Equals( mRotation, 1.f ) )
//		{
//			bIsRemotePlayerRotationOutOfSync = false;
//		}
//	}
//
//	if ( bIsRemotePlayerCameraRotationOutOfSync )
//	{
//		//mLocalActionPawnCameraRotation.Yaw = mLocalRotation.Yaw;
//		//mLocalActionPawnCameraRotation.Roll = mLocalRotation.Roll;
//		mLocalActionPawnCameraRotation = UKismetMathLibrary::RInterpTo(
//			ActionPawnCamera->GetComponentRotation(),
//			mActionPawnCameraRotation,
//			inDeltaTime,
//			BaseLookUpRate * 8.f
//		);
//
//		if ( mLocalActionPawnCameraRotation.Equals( mActionPawnCameraRotation, 1.f ) )
//		{
//			bIsRemotePlayerCameraRotationOutOfSync = false;
//		}
//	}
//
//	A_LOG_1_EXTRA( "~~%%%%%%%%%%%%%%%%%%%SimulateMovementForRemotePawn%%%%%%After%%%%%%%%%%%%%~~~~" );
//
//	A_LOG_M_EXTRA( "READ!!! mLocalRotation = %f, %f, %f", mLocalRotation.Pitch, mLocalRotation.Yaw, mLocalRotation.Roll );
//	A_LOG_1_EXTRA( "------------------------" );
//
//	A_LOG_M_EXTRA( "READ!!! mLocalActionPawnCameraRotation = %f, %f, %f", mLocalActionPawnCameraRotation.Pitch, mLocalActionPawnCameraRotation.Yaw, mLocalActionPawnCameraRotation.Roll );
//	A_LOG_1_EXTRA( "------------------------" );
//
//	A_LOG_M_EXTRA( "READ!!! mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );
//	A_LOG_1_EXTRA( "------------------------" );
//
//	A_LOG_M_EXTRA( "READ!!! mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );
//
//	A_LOG_1_EXTRA( "%%%%%%%%%%%%%%%%%%%~~~SimulateMovementForRemotePawn%%%%%%%%After%%%%%%%%%%%~~~" );
//
//	SetLocalVelocity( mVelocity );
//	SetActorRotation( mLocalRotation );
//	ActionPawnCamera->SetWorldRotation( mLocalActionPawnCameraRotation );
//	SetActorLocation( mLocalLocation );
//
//}

void ARealTimeSrvPawn::SimulateMovementForLocalPawn(float inDeltaTime)
{
	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	A_LOG_M( "READ!!! GetActorRotation() = %f, %f, %f", GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll );
	A_LOG_M( "READ!!! mLocalRotation = %f, %f, %f", mLocalRotation.Pitch, mLocalRotation.Yaw, mLocalRotation.Roll );
	A_LOG_M( "READ!!! GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
	A_LOG_1( "------------------------" );

	A_LOG_M( "READ!!! mLocalActionPawnCameraRotation = %f, %f, %f", mLocalActionPawnCameraRotation.Pitch, mLocalActionPawnCameraRotation.Yaw, mLocalActionPawnCameraRotation.Roll );
	A_LOG_M( "READ!!! ActionPawnCamera->GetComponentRotation() = %f, %f, %f", ActionPawnCamera->GetComponentRotation().Pitch, ActionPawnCamera->GetComponentRotation().Yaw, ActionPawnCamera->GetComponentRotation().Roll );

	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	SetActorRotation( mLocalRotation );
	ActionPawnCamera->SetWorldRotation( mLocalActionPawnCameraRotation );
	if ( bIsPlayerLocationOutOfSync )
	{
		A_LOG_M( "GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
		A_LOG_1( "serverrrrrrrrrrrrrrrrrrr" );

		bIsPlayerLocationOutOfSync = false;
		if ( !GetActorLocation().Equals( mLocation, 100.f ) )
		{
			mLocalLocation = mLocation;
			
			A_MSG_1( 5.f, "drawwwwwwwwwwwwwwwww" );
			A_LOG_1("drawwwwwwwwwwwwwwwww" );
		}
		//else if ( !mVelocity.IsNearlyZero( 1e-6f ) )
		//{
		//	A_LOG_1( "**********************START******************************************" );
		//	A_LOG_1( "!mVelocity.IsNearlyZero( 1e-6f )))))))))))))))))))" );

		//	A_LOG_M( "mLocation = %f, %f, %f", mLocation.X, mLocation.Y, mLocation.Z );
		//	A_LOG_M( "*******before*******, mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );

		//	mLocalLocation = UKismetMathLibrary::VInterpTo( GetActorLocation(), mLocation, inDeltaTime, FMath::Max( mVelocity.Size() / 25.f, 1.f ) );

		//	A_LOG_M( "*******after*******, mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );

		//	//SetActorLocation( mLocalLocation );
		//	//mLocalLocation = GetActorLocation();

		//	if ( !mLocalLocation.Equals( mLocation, 1.f ) )
		//	//if ( mLocalLocation == mLocation )
		//	{
		//		bIsLocalPlayerServerLocationDirty = false;
		//	}

		//	A_LOG_1( "*************************END***************************************" );
		//}
	}
	else
	{
		A_LOG_1( "localllllllllllll" );
		//SetActorLocation( mLocalLocation );
		A_LOG_M( "GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
	}

	SetActorLocation( mLocalLocation );
	SetLocalVelocity( mVelocity );
	 // need to do some lerp
}

void ARealTimeSrvPawn::DR( float inDeltaTime )
{
	FVector Delta = mLocalVelocity * inDeltaTime;
	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		SetLocation( GetActorLocation() + Delta );
	}
}

bool ARealTimeSrvPawn::IsExceedingMaxSpeed( float inMaxSpeed ) const
{
	inMaxSpeed = FMath::Max( 0.f, inMaxSpeed );
	const float MaxSpeedSquared = FMath::Square( inMaxSpeed );

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return ( mVelocity.SizeSquared() > MaxSpeedSquared * OverVelocityPercent );
}

void ARealTimeSrvPawn::ActionAddMovementInput( FVector WorldDirection, float ScaleValue /*= 1.0f*/ )
{
	ActionControlInputVector += WorldDirection * ScaleValue;
}

FVector ARealTimeSrvPawn::ActionConsumeMovementInputVector()
{
	ActionLastControlInputVector = ActionControlInputVector;
	ActionControlInputVector = FVector::ZeroVector;
	return ActionLastControlInputVector;
}

FVector ARealTimeSrvPawn::ActionGetPendingInputVector() const
{
	// There's really no point redirecting to the MovementComponent since GetInputVector is not virtual there, and it just comes back to us.
	return ActionControlInputVector;
}

void ARealTimeSrvPawn::ApplyControlInputToVelocity( float DeltaTime )
{
	const FVector ControlAcceleration = ActionGetPendingInputVector().GetClampedToMaxSize( 1.f );

	const float AnalogInputModifier = ( ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f );
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed( MaxPawnSpeed );

	if ( AnalogInputModifier > 0.f && !bExceedingMaxSpeed )
	{
		// Apply change in velocity direction
		if ( mVelocity.SizeSquared() > 0.f )
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
			mVelocity = mVelocity + ( ControlAcceleration * mVelocity.Size() - mVelocity ) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if ( mVelocity.SizeSquared() > 0.f )
		{
			const FVector OldVelocity = mVelocity;
			const float VelSize = FMath::Max( mVelocity.Size() - FMath::Abs( Deceleration ) * DeltaTime, 0.f );
			mVelocity = mVelocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if ( bExceedingMaxSpeed && mVelocity.SizeSquared() < FMath::Square( MaxPawnSpeed ) )
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

bool ARealTimeSrvPawn::IsFirstPerson() const
{
	return true;
	//return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

void ARealTimeSrvPawn::OnCameraUpdate( const FVector& CameraLocation, const FRotator& CameraRotation )
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

void ARealTimeSrvPawn::Read( InputBitStream& inInputStream )
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

		if ( GetPlayerId() == NetworkMgr::sInstance->GetPlayerId() )
		{
			APlayerController* const FirstPC =  UGameplayStatics::GetPlayerController( GetWorld(), 0 );
			if ( FirstPC != nullptr && !( FirstPC->GetPawn() ) )
			{
				FirstPC->Possess( this );
			}
		}
		else
		{
			mIsTimeToStartSimulateMovementForRemotePawn = 
				NetworkMgr::kTimeBufferStatePackets +
				RealTimeSrvTiming::sInstance.GetCurrentGameTime() -
				NetworkMgr::kTimeBetweenStatePackets;
		}
	}

	FRotator oldRotation = GetActorRotation();
	FRotator oldActionPawnCameraRotation = ActionPawnCamera->GetComponentRotation();
	FVector oldLocation = GetActorLocation();
	FVector oldVelocity = GetLocalVelocity();

	//FRotator oldRotation = mLocalRotation;
	//FRotator oldActionPawnCameraRotation = mLocalActionPawnCameraRotation;
	////FVector oldLocation = GetActorLocation();
	//FVector oldLocation = mLocalLocation;
	//FVector oldVelocity = GetVelocity();


	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	A_LOG_M( "READ!!! GetActorRotation() = %f, %f, %f", GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll );
	A_LOG_M( "READ!!! mLocalRotation = %f, %f, %f", mLocalRotation.Pitch, mLocalRotation.Yaw, mLocalRotation.Roll );
	A_LOG_M( "READ!!! GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
	A_LOG_1( "------------------------" );

	A_LOG_M( "READ!!! mLocalActionPawnCameraRotation = %f, %f, %f", mLocalActionPawnCameraRotation.Pitch, mLocalActionPawnCameraRotation.Yaw, mLocalActionPawnCameraRotation.Roll );
	A_LOG_M( "READ!!! ActionPawnCamera->GetComponentRotation() = %f, %f, %f", ActionPawnCamera->GetComponentRotation().Pitch, ActionPawnCamera->GetComponentRotation().Yaw, ActionPawnCamera->GetComponentRotation().Roll );
	A_LOG_M( "READ!!! mActionPawnCameraRotation = %f, %f, %f", mCameraRotation.Pitch, mCameraRotation.Yaw, mCameraRotation.Roll );
	A_LOG_1( "------------------------" );

	A_LOG_M( "READ!!! GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
	A_LOG_M( "READ!!! mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );
	A_LOG_M( "READ!!! GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );
	A_LOG_1( "------------------------" );

	A_LOG_M( "READ!!! mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
	A_LOG_M( "READ!!! Velocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );

	A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );


	//FRotator replicatedRotation;
	//FVector replicatedLocation;
	//FVector replicatedVelocity;
	//FRotator replicatedActionPawnCameraRotation;

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

		inInputStream.Read( mCameraRotation.Pitch );
		inInputStream.Read( mCameraRotation.Yaw );
		inInputStream.Read( mCameraRotation.Roll );

		//SetActionPawnCameraRotation(mActionPawnCameraRotation);

		readState |= ECRS_Pose;
	}

	if ( GetPlayerId() == NetworkMgr::sInstance->GetPlayerId() )
	{
		ReplayForLocalPawn( readState );

		if ( ( readState & ECRS_PlayerId ) == 0 )
		{
			//InterpolateClientSidePrediction( oldRotation, oldActionPawnCameraRotation, oldLocation, oldVelocity, false );
			PrepareForInterpolate( oldRotation, oldActionPawnCameraRotation, oldLocation, oldVelocity, false );
		}
		else
		{
			InitAfterCreate();
		}
	}
	else
	{
		//// temporary for now, may use entity interp instead later.
		//ReplayForRemotePawn( readState );

		//if ( ( readState & ECRS_PlayerId ) == 0 )
		//{
		//	//InterpolateClientSidePrediction( oldRotation, oldActionPawnCameraRotation, oldLocation, oldVelocity, true );
		//	PrepareForInterpolate( oldRotation, oldActionPawnCameraRotation, oldLocation, oldVelocity, true );
		//}
		//else
		//{
		//	InitAfterCreate();
		//}

		if ( ( readState & ECRS_PlayerId ) != 0 )
		{
			InitAfterCreate();
		}
		else
		{
			mStateBuffer.AddStateData( mRotation, mVelocity, mLocation, mCameraRotation );
		}

		A_LOG_1_EXTRA( "===================mStateBuffer.AddStateData=======start===================" );
		A_LOG_M_EXTRA( "mStateBuffer.AddStateData.mLocation = %f, %f, %f", mLocation.X, mLocation.Y, mLocation.Z );
		A_LOG_M_EXTRA( "mStateBuffer.AddStateData.mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
		A_LOG_M_EXTRA( "mRotation = %f, %f, %f", mRotation.Pitch, mRotation.Yaw, mRotation.Roll );
		A_LOG_M_EXTRA( "mActionPawnCameraRotation = %f, %f, %f", mCameraRotation.Pitch, mCameraRotation.Yaw, mCameraRotation.Roll );
		A_LOG_1_EXTRA( "======end=============mStateBuffer.AddStateData==========================" );
	}

}

void ARealTimeSrvPawn::InitAfterCreate()
{
	mLocalLocation = mLocation;
	mLocalVelocity = mVelocity;
	mLocalRotation = mRotation;
	mLocalActionPawnCameraRotation = mRotation;

	mRemotePawnTargetLocation = mLocation;
	mRemotePawnTargetVelocity = mVelocity;
	mRemotePawnTargetRotation = mRotation;
	mRemotePawnTargetCameraRotation = mRotation;

	SetActorLocation( mLocalLocation );
	SetLocalVelocity( mLocalVelocity );
	SetActorRotation( mLocalRotation );
	ActionPawnCamera->SetWorldRotation( mLocalActionPawnCameraRotation );
}

void ARealTimeSrvPawn::ReplayForLocalPawn( uint32_t inReadState )
{
	if ( ( inReadState & ECRS_Pose ) != 0 )
	{
		const ActionList& moveList = InputManager::sInstance->GetActionList();

		for ( const Action& move : moveList )
		{
			float deltaTime = move.GetDeltaTime();
			ProcessInputBaseOnServerState( deltaTime, move.GetInputState() );
		}
	}
}

void ARealTimeSrvPawn::ReplayForRemotePawn( uint32_t inReadState )
{
	if ( ( inReadState & ECRS_Pose ) != 0 )
	{
		float rtt = NetworkMgr::sInstance->GetRoundTripTime();
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

void ARealTimeSrvPawn::PrepareForInterpolate( const FRotator& inOldRotation, const FRotator& inOldActionPawnCameraRotation, const FVector& inOldLocation, const FVector& inOldVelocity, bool inIsForRemotePawn )
{

	if (inIsForRemotePawn)
	{
		if ( !inOldRotation.Equals( GetRotation(), 1.f ) )
		//if ( inIsForRemotePawn && inOldRotation != GetRotation() )
	{
			bIsRemotePlayerRotationOutOfSync = true;
			bIsRemotePlayerCameraRotationOutOfSync = true;
		}
		else if ( !inOldActionPawnCameraRotation.Equals( GetActionPawnCameraRotation(), 1.f ) )
		{
			bIsRemotePlayerCameraRotationOutOfSync = true;
		}

		//if ( !inOldVelocity.Equals(GetActionEntityVelocity(), 6.f) )
		if ( inOldVelocity != GetLocalVelocity() )
		{
			bIsRemotePlayerVelocityOutOfSync = true;
		}
	}

	//if ( !inOldLocation.Equals( GetLocation(), 10.f ) )
	if ( inOldLocation != GetLocation() )
	{
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "ERROR! Move replay ended with incorrect Location!" );
		A_LOG_M( "inOldLocation = %f, %f, %f", inOldLocation.X, inOldLocation.Y, inOldLocation.Z );
		A_LOG_M( "GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );

		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		bIsPlayerLocationOutOfSync = true;
	}
}

void ARealTimeSrvPawn::InterpolateClientSidePrediction( const FRotator& inOldRotation, const FRotator& inOldActionPawnCameraRotation, const FVector& inOldLocation, const FVector& inOldVelocity, bool inIsForRemotePawn )
{
	if ( !inOldRotation.Equals( GetRotation(), 10.f ) && !inIsForRemotePawn )
	{
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "ERROR! Move replay ended with incorrect local Rotation!" );
		A_LOG_M( "inOldRotation = %f, %f, %f", inOldRotation.Pitch, inOldRotation.Yaw, inOldRotation.Roll );
		A_LOG_M( "GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
		A_LOG_1( "------------------------" );
		A_LOG_M( "inoldActionPawnCameraRotation = %f, %f, %f", inOldActionPawnCameraRotation.Pitch, inOldActionPawnCameraRotation.Yaw, inOldActionPawnCameraRotation.Roll );
		A_LOG_M( "GetActionPawnCameraRotation() = %f, %f, %f", GetActionPawnCameraRotation().Pitch, GetActionPawnCameraRotation().Yaw, GetActionPawnCameraRotation().Roll );
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}

	float roundTripTime = NetworkMgr::sInstance->GetRoundTripTime();
	float time = RealTimeSrvTiming::sInstance.GetFrameStartTime();

	if ( inIsForRemotePawn && !inOldRotation.Equals( GetRotation(), 10.f ) )
		//if ( inIsForRemotePawn && inOldRotation != GetRotation() )
	{
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "ERROR! Move replay ended with incorrect remote Rotation!" );
		A_LOG_M( "inOldRotation = %f, %f, %f", inOldRotation.Pitch, inOldRotation.Yaw, inOldRotation.Roll );
		A_LOG_M( "GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
		A_LOG_1( "------------------------" );
		A_LOG_M( "inoldActionPawnCameraRotation = %f, %f, %f", inOldActionPawnCameraRotation.Pitch, inOldActionPawnCameraRotation.Yaw, inOldActionPawnCameraRotation.Roll );
		A_LOG_M( "GetActionPawnCameraRotation() = %f, %f, %f", GetActionPawnCameraRotation().Pitch, GetActionPawnCameraRotation().Yaw, GetActionPawnCameraRotation().Roll );
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

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

			FRotator newActionPawnCameraRotation = UKismetMathLibrary::RLerp(
				inOldActionPawnCameraRotation,
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


	//if ( !inOldLocation.Equals( GetLocation(), 10.f ) )
	if ( inOldLocation != GetLocation() )
	{
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "ERROR! Move replay ended with incorrect Location!" );
		A_LOG_M( "inOldLocation = %f, %f, %f", inOldLocation.X, inOldLocation.Y, inOldLocation.Z );
		A_LOG_M( "GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );

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
				inIsForRemotePawn ? durationOutOfSync / roundTripTime : 1.f
			);

			A_LOG_M( "newLoc = %f, %f, %f", newLoc.X, newLoc.Y, newLoc.Z );

			SetLocation( newLoc );
			//SetActorLocation( newLoc );
		}
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		bIsPlayerLocationOutOfSync = true;
	}
	else
	{
		mTimeLocationBecameOutOfSync = 0.f;
	}

	//if ( !inOldVelocity.Equals(GetActionEntityVelocity(), 6.f) )
	if ( inIsForRemotePawn && inOldVelocity != GetLocalVelocity() )
	{
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
		A_LOG_1( "ERROR! Move replay ended with incorrect velocity!" );
		A_LOG_M( "inOldVelocity = %f, %f, %f", inOldVelocity.X, inOldVelocity.Y, inOldVelocity.Z );
		A_LOG_M( "GetVelocity() = %f, %f, %f", GetLocalVelocity().X, GetLocalVelocity().Y, GetLocalVelocity().Z );


		if ( mTimeVelocityBecameOutOfSync == 0.f )
		{
			mTimeVelocityBecameOutOfSync = time;
		}

		float durationOutOfSync = time - mTimeVelocityBecameOutOfSync;
		if ( durationOutOfSync < roundTripTime )
		{

			FVector newVel = UKismetMathLibrary::VLerp(
				inOldVelocity,
				GetLocalVelocity(),
				inIsForRemotePawn ? durationOutOfSync / roundTripTime : 1.f
			);

			A_LOG_M( "newVel = %f, %f, %f", newVel.X, newVel.Y, newVel.Z );

			SetLocalVelocity( newVel );
		}
		A_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );
	}
	else
	{
		//A_LOG_1( " RESET mTimeVelocityBecameOutOfSync!!!" );
		mTimeVelocityBecameOutOfSync = 0.f;
	}
}


