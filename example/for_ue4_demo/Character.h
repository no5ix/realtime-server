#pragma once

#include <realtime_srv/RealtimeServer.h>


class Character : public realtime_srv::GameObj
{
public:
	Character();

	// 'CHRT' = 1128813140;
	CLASS_IDENTIFICATION( 1128813140 );

	enum EReplicationState
	{
		EPS_Pose = 1 << 0,
		EPS_PlayerId = 1 << 1,

		EPS_AllState = EPS_Pose | EPS_PlayerId
	};
	virtual uint32_t GetAllStateMask() const override { return EPS_AllState; }

	virtual uint32_t Write( realtime_srv::OutputBitStream& inOutputStream,
		uint32_t inDirtyState ) const override;

public:
	static Character	StaticCreate() 
	{ Character* toRet = new Character(); return *toRet; }

	float GetMaxSpeed() const { return MaxSpeed; }
	const realtime_srv::Vector3& GetCameraRotation() const { return curCameraRotation_; }
	const realtime_srv::Vector3& GetVelocity() const { return currentVelocity_; }

	void SetPlayerId( int newPlayerId ) { playerId_ = newPlayerId; }

protected:

	virtual void Update() override;
	virtual void SetOldState() override;
	virtual bool IsStateDirty() override;
	virtual void ProcessInput( float inDeltaTime,
		const realtime_srv::InputState& inInputState ) override;

protected:
	void HandleShooting() {}

	/** Update Velocity based on input. Also applies gravity. */
	void ApplyControlInputToVelocity( float DeltaTime );

	bool IsExceedingMaxSpeed( float inMaxSpeed ) const;
	void ActionAddMovementInput( const realtime_srv::Vector3& WorldDirection, float ScaleValue = 1.0f );
	const realtime_srv::Vector3& ConsumeMovementInputVector();
	const realtime_srv::Vector3& GetPendingInputVector() const;

private:

	realtime_srv::Vector3 ControlInputVector_;

	realtime_srv::Vector3 LastControlInputVector_;

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

	realtime_srv::Vector3 curCameraRotation_;
	realtime_srv::Vector3 currentVelocity_;
	realtime_srv::Vector3 oldCameraRotation_;
	realtime_srv::Vector3 oldrentVelocity_;

	int playerId_;
};

typedef shared_ptr< Character >	CharacterPtr;