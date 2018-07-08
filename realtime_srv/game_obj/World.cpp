#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;



int World::kNewObjId = 1;

World::World() 
{}

void World::Registry( GameObjPtr inGameObject, ReplicationAction inAction )
{
	if ( inAction == RA_Create )
	{
		RegistGameObj( inGameObject );
	}
	else if ( inAction == RA_Destroy )
	{
		UnregistGameObj( inGameObject );
	}
}

int World::GetNewObjId()
{
	int toRet = kNewObjId++;
	if ( kNewObjId < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long..." );
	}
	return toRet;
}

GameObjPtr World::GetGameObject( int inObjId )
{
	auto gameObjectIt = ObjIdToGameObjMap_.find( inObjId );
	if ( gameObjectIt != ObjIdToGameObjMap_.end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return GameObjPtr();
	}
}

void World::RegistGameObj( GameObjPtr inGameObject )
{
	int newObjId = GetNewObjId();
	inGameObject->SetObjId( newObjId );
	ObjIdToGameObjMap_[newObjId] = inGameObject;

	notifyAllClientCB_( inGameObject, RA_Create );
	auto newClientProxy = inGameObject->GetClientProxy();
	if ( newClientProxy )
	{
		newClientProxy->SetWorld( this );
		for ( const auto& pair : ObjIdToGameObjMap_ )
		{
			newClientProxy->GetReplicationManager().ReplicateCreate(
				pair.first, pair.second->GetAllStateMask() );
		}
	}
}

void World::UnregistGameObj( GameObjPtr inGameObject )
{
	notifyAllClientCB_( inGameObject, RA_Destroy );

	int objId = inGameObject->GetObjId();
	ObjIdToGameObjMap_.erase( objId );
}

void World::Update()
{
	vector< GameObjPtr > GameObjsToRem;
	for ( const auto& pair : ObjIdToGameObjMap_ )
	{
		auto go = pair.second;

		if ( !go->DoesWantToDie() )
		{
			go->Update();
		}
		else
		{
			GameObjsToRem.push_back( go );
		}
	}

	if ( GameObjsToRem.size() > 0 )
	{
		for ( auto& g : GameObjsToRem )
			UnregistGameObj( g );
	}
}
