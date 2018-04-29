// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "ActionEntity.h"
#include "ActionInputState.h"
#include "ActionPawn.generated.h"

UCLASS()
class AActionPawn : public AActionEntity
{
	GENERATED_UCLASS_BODY()

public:

	// 'CHRT' = 1128813140;
	virtual uint32_t GetClassId() const /*{ return 1128813140; }*/ ;

protected:

	/** Update Velocity based on input. Also applies gravity. */
	void ApplyControlInputToVelocity(float DeltaTime);

public:

	virtual void	Update() override;

	void LocalSimulateMovement( float inDeltaTime, const ActionInputState& inInputState );

	virtual void	Read( InputMemoryBitStream& inInputStream ) override;

	void ProcessInput( float inDeltaTime, const ActionInputState& inInputState );

	void SimulateMovementAfterReplay();

	void DR( float inDeltaTime );
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	/** setup pawn specific input handlers */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/**
	* Move forward/back
	*
	* @param Val Movment input to apply
	*/
	void MoveForward(float Val);

	/**
	* Strafe right/left
	*
	* @param Val Movment input to apply
	*/
	void MoveRight(float Val);

	/**
	* Move Up/Down in allowed movement modes.
	*
	* @param Val Movment input to apply
	*/
	//void MoveUp(float Val);

	void Turn( float Val );

	void LookUp( float Val );

	bool IsExceedingMaxSpeed( float inMaxSpeed ) const;

	void ActionAddMovementInput( FVector WorldDirection, float ScaleValue = 1.0f );

	FVector ActionConsumeMovementInputVector();

	FVector ActionGetPendingInputVector() const;

	/**
	* Add camera pitch to first person mesh.
	*
	*	@param	CameraLocation	Location of the Camera.
	*	@param	CameraRotation	Rotation of the Camera.
	*/
	void OnCameraUpdate( const FVector& CameraLocation, const FRotator& CameraRotation );

	/** get camera view type */
	UFUNCTION( BlueprintCallable, Category = Mesh )
		virtual bool IsFirstPerson() const;

	//FRotator GetLocalActionPawnCameraRotation() const { return mLocalActionPawnCameraRotation; }

	FRotator GetActionPawnCameraRotation() const { return mActionPawnCameraRotation; }
	void SetActionPawnCameraRotation( FRotator inActionPawnCameraRotation ) { mActionPawnCameraRotation = inActionPawnCameraRotation; }

public:

	void ReplayForLocalPawn( uint32_t inReadState );
	void ReplayForRemotePawn( uint32_t inReadState );


private:

	void InterpolateClientSidePrediction( const FRotator& inOldRotation, const FRotator& inOldActionPawnCameraRotation, const FVector& inOldLocation, const FVector& inOldVelocity, bool inIsForRemotePawn );

public:

	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	FORCEINLINE class USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

	FORCEINLINE class UCameraComponent* GetCamera() const { return ActionPawnCamera; }


	float GetMaxSpeed() const { return MaxSpeed; }
private:

	/** Camera component that will be our viewpoint */
	UPROPERTY( VisibleDefaultsOnly, Category = ActionPawnCamera )
		class UCameraComponent* ActionPawnCamera;

	/** pawn mesh: 1st person view */
	UPROPERTY( VisibleDefaultsOnly, Category = Mesh )
		USkeletalMeshComponent* Mesh1P;

	/** pawn mesh: 1st person view */
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
	FRotator mActionPawnCameraRotation;
	FRotator mLocalActionPawnCameraRotation;

	FRotator mLocalRotation;

protected:

	float	mLastDragTimestamp;

	float	mTimeLocationBecameOutOfSync;
	float	mTimeVelocityBecameOutOfSync;
	float	mTimeRotationBecameOutOfSync;
};
