#pragma once
#include <memory>
#include <unordered_map>
#include <functional>
#include "realtime_srv/common/noncopyable.h"
#include "realtime_srv/rep/ReplicationCmd.h"


namespace realtime_srv
{

class GameObj;
class ClientProxy;

class World : noncopyable, public std::enable_shared_from_this<World>
{
public:
	typedef std::unordered_map< int, std::shared_ptr<GameObj> > ObjIdToGameObjMap;

	typedef std::function<void(
		std::shared_ptr<GameObj>&, ReplicationAction )> OnObjCreateOrDestoryCb;

	void Update();

	void OnObjCreateOrDestoryCallback( const OnObjCreateOrDestoryCb& _cb )
	{ onObjCreateOrDestoryCb_ = _cb; }

	bool IsGameObjectExist( int _objId );
	std::shared_ptr<GameObj> GetGameObject( int _objId );

	ObjIdToGameObjMap& GetAllGameObj() { return ObjIdToGameObjMap_; }
	const ObjIdToGameObjMap& GetAllGameObj() const { return ObjIdToGameObjMap_; }

	void Registry( std::shared_ptr<GameObj> _obj, ReplicationAction _action );
	void RegistGameObj( std::shared_ptr<GameObj> _obj );
	void UnregistGameObj( std::shared_ptr<GameObj> _obj );

	void WhenClientProxyHere( std::shared_ptr<ClientProxy> _cliProxy );

private:
	int	GetNewObjId();

private:
	OnObjCreateOrDestoryCb onObjCreateOrDestoryCb_;

	ObjIdToGameObjMap		ObjIdToGameObjMap_;
	static int					kNewObjId;

};
typedef std::shared_ptr<World> WorldPtr;
}