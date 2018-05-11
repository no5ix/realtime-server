// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "RealTimeSrvEntity.h"
#include "RealTimeSrvInputState.h"
#include "StateQueue.h"
#include "RealTimeSrvPawn.generated.h"

UCLASS()
class ARealTimeSrvPawn : public ARealTimeSrvEntity
{
	GENERATED_UCLASS_BODY()

public:

	// 'CHRT' = 1128813140;
	virtual uint32_t GetClassId() const /*{ return 1128813140; }*/ ;

protected:

	void ApplyControlInputToVelocity( float DeltaTime, FVector& inVelocity );

public:

	virtual void	Update() override;

	void ProcessInputBaseOnLocalState( float inDeltaTime, const RealTimeSrvInputState& inInputState );

	virtual void	Read( InputBitStream& inInputStream ) override;

	void InitAfterCreate();

	void ProcessInputBaseOnServerState( float inDeltaTime, const RealTimeSrvInputState& inInputState );

	void SimulateMovementForRemotePawn(float inDeltaTime);

	void SimulateMovementForLocalPawn(float inDeltaTime);

	void DR( float inDeltaTime );
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	void MoveForward(float Val);

	void MoveRight(float Val);

	//void MoveUp(float Val);

	void Turn( float Val );

	void LookUp( float Val );

	bool IsExceedingMaxSpeed( float inMaxSpeed ) const;

	void ActionAddMovementInput( FVector WorldDirection, float ScaleValue = 1.0f );

	FVector ActionConsumeMovementInputVector();

	FVector ActionGetPendingInputVector() const;

	virtual void	UpdateTargetState() override;

public:

	void ReplayForLocalPawn( uint32_t inReadState );
	void ReplayForRemotePawn( uint32_t inReadState );

public:

	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	FORCEINLINE class USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

	FORCEINLINE class UCameraComponent* GetCamera() const { return RealTimeSrvPawnCamera; }


	float GetMaxSpeed() const { return MaxSpeed; }
private:

	UPROPERTY( VisibleDefaultsOnly, Category = RealTimeSrvPawnCamera )
		class UCameraComponent* RealTimeSrvPawnCamera;

	UPROPERTY( VisibleDefaultsOnly, Category = Mesh )
		USkeletalMeshComponent* Mesh1P;

	UPROPERTY( VisibleDefaultsOnly, Category = Mesh )
		USkeletalMeshComponent* Mesh3P;

private:

	FVector ActionControlInputVector;

	FVector ActionLastControlInputVector;
protected:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
		bool bTestUpdateForDisconnect;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
	float BaseTurnRate;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
	float BaseLookUpRate;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
		float MaxSpeed;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
		float Acceleration;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement )
		float Deceleration;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPawnMovement, meta = ( ClampMin = "0", UIMin = "0" ) )
		float TurningBoost;

protected:
	FRotator mCameraRotation;
	FRotator mLocalCameraRotation;

	bool bIsPlayerLocationOutOfSync;



protected:

	StateQueue mStateBuffer;

	FVector mRemotePawnTargetLocation;
	FVector mRemotePawnTargetVelocity;
	FRotator mRemotePawnTargetRotation;
	FRotator mRemotePawnTargetCameraRotation;

	float mTimeOfLastUpdateTargetState;
};
