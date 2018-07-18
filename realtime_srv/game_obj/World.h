#pragma once


namespace realtime_srv
{

class GameObj;

class World : noncopyable, public std::enable_shared_from_this<World>
{
public:
	typedef unordered_map< int, GameObjPtr > ObjIdToGameObjMap;
	typedef std::function<void( GameObjPtr&, ReplicationAction )> OnObjCreateOrDestoryCb;

	void Update();

	void OnObjCreateOrDestoryCallback( const OnObjCreateOrDestoryCb& cb )
	{ onObjCreateOrDestoryCb_ = cb; }

	bool IsGameObjectExist( int inObjId );
	GameObjPtr GetGameObject( int inNetworkId );

	ObjIdToGameObjMap& GetAllGameObj() { return ObjIdToGameObjMap_; }
	const ObjIdToGameObjMap& GetAllGameObj() const { return ObjIdToGameObjMap_; }

	void Registry( GameObjPtr inGameObject, ReplicationAction inAction );
	void RegistGameObj( GameObjPtr inGameObject );
	void UnregistGameObj( GameObjPtr inGameObject );

private:
	int	GetNewObjId();

private:
	OnObjCreateOrDestoryCb onObjCreateOrDestoryCb_;

	ObjIdToGameObjMap		ObjIdToGameObjMap_;
	static int					kNewObjId;

};
typedef shared_ptr<World> WorldPtr;
}