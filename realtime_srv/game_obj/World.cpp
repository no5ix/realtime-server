#include "realtime_srv/common/RealtimeSrvHelper.h"
#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/net/ClientProxy.h"

#include "realtime_srv/game_obj/World.h"


using namespace realtime_srv;



int World::kNewObjId = 1;

void World::Registry( GameObjPtr _obj, ReplicationAction _repAction )
{
	if ( _repAction == RA_Create )
		return RegistGameObj( _obj );
	else if ( _repAction == RA_Destroy )
		return UnregistGameObj( _obj );
}

void World::RegistGameObj( GameObjPtr _obj )
{
	int newObjId = GetNewObjId();
	_obj->SetObjId( newObjId );
	_obj->SetWorld( shared_from_this() );
	ObjIdToGameObjMap_[newObjId] = _obj;

	onObjCreateOrDestoryCb_( _obj, RA_Create );
}

void World::UnregistGameObj( GameObjPtr _obj )
{
	onObjCreateOrDestoryCb_( _obj, RA_Destroy );
	ObjIdToGameObjMap_.erase( _obj->GetObjId() );
}

void World::WhenClientProxyHere( std::shared_ptr<ClientProxy> _cliProxy )
{
	if ( _cliProxy )
	{
		_cliProxy->SetWorld( shared_from_this() );
		for ( const auto& ipair : ObjIdToGameObjMap_ )
		{
			_cliProxy->GetReplicationMgr().ReplicateCreate(
				ipair.first, ipair.second->GetAllStateMask() );
		}
	}
}

int World::GetNewObjId()
{
	int toRet = kNewObjId++;
	if ( kNewObjId < toRet )
		LOG( "GameObj ID Wrap Around!!! You've been playing way too long..." );
	return toRet;
}

bool World::IsGameObjectExist( int _objId )
{
	auto gameObjectIt = ObjIdToGameObjMap_.find( _objId );
	return gameObjectIt != ObjIdToGameObjMap_.end();
}

GameObjPtr World::GetGameObject( int _objId )
{
	auto gameObjectIt = ObjIdToGameObjMap_.find( _objId );
	if ( gameObjectIt != ObjIdToGameObjMap_.end() )
		return gameObjectIt->second;
	else
		return GameObjPtr();
}

void World::Update()
{
	for ( auto it = ObjIdToGameObjMap_.begin();
		it != ObjIdToGameObjMap_.end(); )
	{
		if ( it->second->IsPendingToDie() )
			UnregistGameObj( it++->second );
		else
			it++->second->Update();
	}
}
