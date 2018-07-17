#pragma once

namespace realtime_srv
{

#define CLASS_IDENTIFICATION( inCode ) \
enum { kClassId = inCode }; \
virtual uint32_t GetClassId() const { return kClassId; } \

class ClientProxy;
class InputState;

class GameObj
{
public:
	// 'GOBJ' = 1196376650;
	CLASS_IDENTIFICATION( 1196376650 );

	enum EReplicationState
	{
		EPS_Pose = 1 << 0,
		EPS_AllState = EPS_Pose
	};
	virtual uint32_t GetAllStateMask() const { return EPS_AllState; }

	GameObj();
	virtual ~GameObj() {}

	virtual void Update();

	virtual uint32_t	Write( OutputBitStream& inOutputStream,
		uint32_t inDirtyState ) const = 0;

	virtual void Read( InputBitStream& inInputStream ) {}

public:

	shared_ptr< ClientProxy >	GetClientProxy() const { return clientProxy_.lock(); }
	void SetClientProxy( shared_ptr< ClientProxy > cp ) { clientProxy_ = cp; }

	bool		IsPendingToDestroy() const { return isPendingToDestroy_; }
	void		SetPendingToDestroy( bool whether ) { isPendingToDestroy_ = whether; }

	int			GetObjId()				const { return ObjId_; }
	void		SetObjId( int inObjId ) { ObjId_ = inObjId; }

protected:

	virtual void SetOldState() = 0;
	virtual void CheckAndSetDirtyState() = 0;

	virtual void ProcessInput( float inDeltaTime,
		const std::shared_ptr<InputState>& inInputState ) = 0;

	void SetStateDirty( uint32_t repState );

protected:

	bool isPendingToDestroy_;

	int	ObjId_;

	weak_ptr<ClientProxy> clientProxy_;
};

typedef shared_ptr< GameObj >	GameObjPtr;

}