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

	void OnObjCreateOrDestoryCallback( const OnObjCreateOrDestoryCb& _cb )
	{ onObjCreateOrDestoryCb_ = _cb; }

	bool IsGameObjectExist( int _objId );
	GameObjPtr GetGameObject( int _objId );

	ObjIdToGameObjMap& GetAllGameObj() { return ObjIdToGameObjMap_; }
	const ObjIdToGameObjMap& GetAllGameObj() const { return ObjIdToGameObjMap_; }

	void Registry( GameObjPtr _obj, ReplicationAction _action );
	void RegistGameObj( GameObjPtr _obj );
	void UnregistGameObj( GameObjPtr _obj );

private:
	int	GetNewObjId();

private:
	OnObjCreateOrDestoryCb onObjCreateOrDestoryCb_;

	ObjIdToGameObjMap		ObjIdToGameObjMap_;
	static int					kNewObjId;

};
typedef shared_ptr<World> WorldPtr;
}