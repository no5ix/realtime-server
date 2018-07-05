#pragma once


namespace realtime_srv
{

class GameObj;

class World : noncopyable
{
public:
	typedef unordered_map< int, GameObjPtr > ObjIdToGameObjMap;
	typedef std::shared_ptr< unordered_map< int, GameObjPtr > > ObjIdToGameObjMapPtr;
	typedef std::function<void( GameObjPtr, ReplicationAction )> NotifyAllClientCB;

	World();
	void Update();

	void SetNotifyAllClientCallback( const NotifyAllClientCB& cb ) { notifyAllClientCB_ = cb; }

	GameObjPtr GetGameObject( int inNetworkId );
	void Registry( GameObjPtr inGameObject, ReplicationAction inAction );
	void RegistGameObj( GameObjPtr inGameObject );
	void UnregistGameObj( GameObjPtr inGameObject );

private:
	int	GetNewObjId();

private:
	NotifyAllClientCB notifyAllClientCB_;

	ObjIdToGameObjMap			ObjIdToGameObjMap_;
	static int					kNewObjId;

};
}