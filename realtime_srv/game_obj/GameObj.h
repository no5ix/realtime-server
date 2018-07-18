#pragma once

namespace realtime_srv
{

#define CLASS_IDENTIFICATION( inCode ) \
enum { kClassId = inCode }; \
virtual uint32_t GetClassId() const { return kClassId; } \

class ClientProxy;
class InputState;
class World;
class NetworkMgr;

class GameObj : public std::enable_shared_from_this<GameObj>
{
public:
	// 'GOBJ' = 1196376650;
	CLASS_IDENTIFICATION( 1196376650 );

	enum EReplicationState
	{
		ERS_Pose = 1 << 0,
		EPS_AllState = ERS_Pose
	};
	virtual uint32_t GetAllStateMask() const { return EPS_AllState; }

	GameObj();
	virtual ~GameObj() {}

	virtual void Update();

	virtual void Read( InputBitStream& inInputStream ) {}
	virtual uint32_t	Write( OutputBitStream& inOutputStream,
		uint32_t inDirtyState ) const = 0;

	std::shared_ptr<ClientProxy>	GetOwner() { return owner_.lock(); }
	void LoseOwner() { hasOwner_ = false; }
	void SetOwner( std::shared_ptr<ClientProxy>& cp )
	{ owner_ = cp; hasOwner_ = true; }

	void SetWorld( const std::shared_ptr<World>& _world ) { world_ = _world; }

	void SetNetworkMgr( const std::shared_ptr<NetworkMgr>& _networkMgr )
	{ networkMgr_ = _networkMgr; }

	bool		IsPendingToDie() const { return isPendingToDie_; }
	void		SetPendingToDie( bool whether ) { isPendingToDie_ = whether; }

	int			GetObjId()				const { return objId_; }
	void		SetObjId( int inObjId ) { objId_ = inObjId; }

protected:

	virtual void BeforeProcessInput() {}
	virtual void AfterProcessInput() = 0;

	virtual void ProcessInput( float inDeltaTime,
		const std::shared_ptr<InputState>& inInputState ) {}

	void SetStateDirty( uint32_t repState );

protected:

	bool isPendingToDie_;
	bool hasOwner_;

	int	objId_;

	std::weak_ptr<ClientProxy> owner_;
	std::shared_ptr<World> world_;
	std::shared_ptr<NetworkMgr> networkMgr_;
};

typedef std::shared_ptr< GameObj >	GameObjPtr;

}