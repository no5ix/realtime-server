// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ActionEntity.h"
#include "GameFramework/Character.h"
#include "ActionCharacter.generated.h"


UCLASS()
class AActionCharacter : public AActionEntity
{
	GENERATED_UCLASS_BODY()

public:

	//CLASS_IDENTIFICATION( 'CHAR', GameObject )

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
	void MoveUp(float Val);

	void Turn( float Val );

	void LookUp( float Val );

	/* Frame rate independent turn */
	void TurnAtRate(float Val);

	/* Frame rate independent lookup */
	void LookUpAtRate( float Val );

	/** player pressed jump action */
	void OnStartJump();

	/** player released jump action */
	void OnStopJump();
	
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

private:

	/** pawn mesh: 1st person view */
	UPROPERTY( VisibleDefaultsOnly, Category = Mesh )
		USkeletalMeshComponent* Mesh1P;

protected:


	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	float BaseTurnRate;

	/** Base lookup rate, in deg/sec. Other scaling may affect final lookup rate. */
	float BaseLookUpRate;
};
