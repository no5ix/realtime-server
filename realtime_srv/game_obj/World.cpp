#include "realtime_srv/common/RealtimeSrvShared.h"


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
	auto newClientProxy = _obj->GetOwner();
	if ( newClientProxy )
	{
		newClientProxy->SetWorld( shared_from_this() );
		for ( const auto& ipair : ObjIdToGameObjMap_ )
		{
			newClientProxy->GetReplicationManager().ReplicateCreate(
				ipair.first, ipair.second->GetAllStateMask() );
		}
	}
}

void World::UnregistGameObj( GameObjPtr _obj )
{
	onObjCreateOrDestoryCb_( _obj, RA_Destroy );
	_obj->WhenDying();
	ObjIdToGameObjMap_.erase( _obj->GetObjId() );
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
	vector< GameObjPtr > GameObjsToRem;
	for ( const auto& pair : ObjIdToGameObjMap_ )
	{
		auto curObj = pair.second;
		if ( curObj->IsPendingToDie() )
			GameObjsToRem.push_back( curObj );
		else
			curObj->Update();
	}

	for ( auto& obj : GameObjsToRem ) UnregistGameObj( obj );
}
