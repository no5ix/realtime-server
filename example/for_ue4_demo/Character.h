#include <realtime_srv/RealtimeServer.h>


class Character : public GameObj
{
public:
	// 'CHRT' = 1128813140;
	CLASS_IDENTIFICATION( 1128813140, GameObj );

	enum EReplicationState
	{
		EPS_Pose = 1 << 0,
		EPS_PlayerId = 1 << 1,

		EPS_AllState = EPS_Pose
		| EPS_PlayerId
	};
	virtual uint32_t GetAllStateMask() const override { return EPS_AllState; }

	virtual uint32_t Write( OutputBitStream& inOutputStream,
		uint32_t inDirtyState ) const override;

public:
	static GameObjPtr	StaticCreate()
	{ GameObjPtr toRet( new Character() ); return toRet; }

	float GetMaxSpeed() const { return MaxSpeed; }
	const Vector3& GetCameraRotation() const { return curCameraRotation_; }
	const Vector3& GetVelocity() const { return currentVelocity_; }

protected:
	Character();

	virtual void Update() override;
	virtual void SetOldState() override;
	virtual bool IsStateDirty() override;
	virtual void ProcessInput( float inDeltaTime, const InputState& inInputState ) override;

protected:
	void HandleShooting() {}

	/** Update Velocity based on input. Also applies gravity. */
	void ApplyControlInputToVelocity( float DeltaTime );

	bool IsExceedingMaxSpeed( float inMaxSpeed ) const;
	void ActionAddMovementInput( Vector3 WorldDirection, float ScaleValue = 1.0f );
	Vector3 ActionConsumeMovementInputVector();
	Vector3 ActionGetPendingInputVector() const;

private:

	Vector3 ActionControlInputVector;

	Vector3 ActionLastControlInputVector;

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

	Vector3 curCameraRotation_;
	Vector3 currentVelocity_;
	Vector3 oldCameraRotation_;
	Vector3 oldrentVelocity_;


};

typedef shared_ptr< Character >	CharacterPtr;