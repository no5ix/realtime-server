#pragma once


namespace realtime_srv
{

class GameObj;

class World : noncopyable
{
public:
	typedef unordered_map< int, GameObjPtr > ObjIdToGameObjMap;
	typedef std::function<void( GameObjPtr&, ReplicationAction )> NotifyAllClientCB;

	void Update();

	void SetNotifyAllClientCallback( const NotifyAllClientCB& cb )
	{ notifyAllClientCB_ = cb; }

	bool IsGameObjectExist( int inObjId );
	GameObjPtr GetGameObject( int inNetworkId );

	ObjIdToGameObjMap& GetAllGameObj() { return ObjIdToGameObjMap_; }
	const ObjIdToGameObjMap& GetAllGameObj() const { return ObjIdToGameObjMap_; }

	void Registry( GameObjPtr& inGameObject, ReplicationAction inAction );
	void RegistGameObj( GameObjPtr& inGameObject );
	void UnregistGameObj( GameObjPtr& inGameObject );

private:
	int	GetNewObjId();

private:
	NotifyAllClientCB notifyAllClientCB_;

	ObjIdToGameObjMap		ObjIdToGameObjMap_;
	static int					kNewObjId;

};
}