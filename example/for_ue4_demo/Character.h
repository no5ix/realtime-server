#pragma once

#include <realtime_srv/RealtimeServer.h>


class Character : public realtime_srv::GameObj
{
public:
	Character();

	virtual ~Character()
	{ realtime_srv::LOG("Character (holded by Player %d, ObjId=%d) die.", playerId_, objId_); }

	// 'CHRT' = 1128813140;
	CLASS_IDENTIFICATION(1128813140);

	enum EReplicationState
	{
		ERS_Pose = 1 << 0,
		ERS_PlayerId = 1 << 1,

		EPS_AllState = ERS_Pose | ERS_PlayerId
	};
	virtual uint32_t GetAllStateMask() const override { return EPS_AllState; }

	virtual uint32_t Write(realtime_srv::OutputBitStream& outputStream,
		uint32_t dirtyState) const override;

public:

	const realtime_srv::Vector3&	GetRotation() const { return curRotation_; }
	void SetRotation(realtime_srv::Vector3 inRotation) { curRotation_ = inRotation; }
	void SetRotation(const float x, const float y, const float z)
	{ curRotation_ = realtime_srv::Vector3(x, y, z); }

	const realtime_srv::Vector3&	GetLocation() const { return curLocation_; }
	void SetLocation(const realtime_srv::Vector3& inLocation) { curLocation_ = inLocation; }
	void SetLocation(const float x, const float y, const float z)
	{ curLocation_ = realtime_srv::Vector3(x, y, z); }

	float GetMaxSpeed() const { return maxSpeed_; }
	const realtime_srv::Vector3& GetCameraRotation() const { return curCameraRotation_; }
	const realtime_srv::Vector3& GetVelocity() const { return curVelocity_; }

	void SetPlayerId(int newPlayerId) { playerId_ = newPlayerId; }
	int GetPlayerId() const { return playerId_; }

protected:

	virtual void BeforeProcessInput() override;
	virtual void AfterProcessInput() override;

	virtual void ProcessInput(float deltaTime,
		const realtime_srv::InputStatePtr& inputState) override;

protected:
	void HandleShooting() {}

	/** Update Velocity based on input. Also applies gravity. */
	void ApplyControlInputToVelocity(float deltaTime);

	bool IsExceedingMaxSpeed(float maxSpeed) const;
	void AddActionInput(const realtime_srv::Vector3& WorldDirection, float ScaleValue = 1.0f);
	const realtime_srv::Vector3& ConsumeMovementInputVector();
	const realtime_srv::Vector3& GetPendingInputVector() const;

protected:

	realtime_srv::Vector3 ControlInputVector_;

	realtime_srv::Vector3 LastControlInputVector_;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	float baseTurnRate_;

	/** Base lookup rate, in deg/sec. Other scaling may affect final lookup rate. */
	float baseLookUpRate_;

	/** Maximum velocity magnitude allowed for the controlled Pawn. */
	float maxSpeed_;

	/** Acceleration applied by input (rate of change of velocity) */
	float acceleration_;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	float deceleration_;

	float turningBoost_;


	realtime_srv::Vector3 curLocation_;
	realtime_srv::Vector3 oldLocation_;

	realtime_srv::Vector3 curRotation_;
	realtime_srv::Vector3 oldRotation_;

	realtime_srv::Vector3 curCameraRotation_;
	realtime_srv::Vector3 curVelocity_;
	realtime_srv::Vector3 oldCameraRotation_;
	realtime_srv::Vector3 oldVelocity_;

	int playerId_;
};

typedef std::shared_ptr< Character >	CharacterPtr;