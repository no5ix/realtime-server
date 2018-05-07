#pragma once

class Character : public Entity
{
public:
	// 'CHRT' = 1128813140;
	CLASS_IDENTIFICATION( 'CHRT', Entity )

	enum ECharacterReplicationState
	{
		ECRS_Pose = 1 << 0,
		ECRS_PlayerId = 1 << 2,
		//ECRS_Health = 1 << 3,

		ECRS_AllState = ECRS_Pose 
			| ECRS_PlayerId
			//| ECRS_Health
	};


	static	Entity*	StaticCreate() { return new Character(); }

	virtual uint32_t GetAllStateMask()	const override { return ECRS_AllState; }


	virtual void Update() override;

	void ProcessInput( float inDeltaTime, const InputState& inInputState );
	void SimulateMovement( float inDeltaTime );


	void		SetPlayerId( uint32_t inPlayerId ) { mPlayerId = inPlayerId; }
	uint32_t	GetPlayerId()						const { return mPlayerId; }

	virtual uint32_t	Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const override;


	bool IsExceedingMaxSpeed( float inMaxSpeed ) const;

	void ActionAddMovementInput( Vector3 WorldDirection, float ScaleValue = 1.0f );

	Vector3 ActionConsumeMovementInputVector();

	Vector3 ActionGetPendingInputVector() const;

	virtual Vector3 GetVelocity() const { return Velocity; }

	void			SetVelocity( const Vector3& inVelocity ) { Velocity = inVelocity; }

	float GetMaxSpeed() const { return MaxSpeed; }

	Vector3 GetActionPawnCameraRotation() const { return ActionPawnCameraRotation; }

//public:
protected:

	/** Update Velocity based on input. Also applies gravity. */
	void ApplyControlInputToVelocity( float DeltaTime );


	Vector3 ActionControlInputVector;

	Vector3 ActionLastControlInputVector;

	Character();

private:


	uint32_t			mPlayerId;

protected:

	///move down here for padding reasons...

	float				mLastMoveTimestamp;

	int					mHealth;

	bool				mIsShooting;

	Vector3             ActionPawnCameraRotation;

protected:


	Vector3 Velocity;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */

	float BaseTurnRate;

	/** Base lookup rate, in deg/sec. Other scaling may affect final lookup rate. */

	float BaseLookUpRate;

	/** Maximum velocity magnitude allowed for the controlled Pawn. */

	float MaxSpeed;

	/** Acceleration applied by input (rate of change of velocity) */

	float Acceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */

	float Deceleration;


	float TurningBoost;
};

typedef shared_ptr< Character >	CharacterPtr;