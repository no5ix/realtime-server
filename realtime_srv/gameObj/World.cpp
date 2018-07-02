#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;


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


#ifdef IS_LINUX


using namespace muduo;

AtomicInt32 World::kNewObjId;

World::World() :
	ObjIdToGameObjMap_( new ObjIdToGameObjMap )
{
	kNewObjId.getAndSet( 1 );
}

GameObjPtr World::GetGameObject( int inObjId )
{
	auto tempObjIdToGameObjMap = GET_THREAD_SHARED_VAR( ObjIdToGameObjMap_ );
	auto gameObjectIt = tempObjIdToGameObjMap->find( inObjId );
	if ( gameObjectIt != tempObjIdToGameObjMap->end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return GameObjPtr();
	}
}

int World::GetNewObjId()
{
	int toRet = kNewObjId.getAndAdd( 1 );
	if ( kNewObjId.get() < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long..." );
	}

	return toRet;
}

void World::RegistGameObj( GameObjPtr inGameObject )
{
	int newObjId = GetNewObjId();
	inGameObject->SetObjId( newObjId );

	auto regFunc = [newObjId, &inGameObject, this]() { ( *ObjIdToGameObjMap_ )[newObjId] = inGameObject; };
	SET_THREAD_SHARED_VAR( ObjIdToGameObjMap_, mutex_, regFunc );

	notifyAllClientCB_( inGameObject, RA_Create );
	auto newClientProxy = inGameObject->GetClientProxy();
	if ( newClientProxy )
	{
		newClientProxy->SetWorld( this );
		ObjIdToGameObjMapPtr tempObjIdToGameObjMap =
			GET_THREAD_SHARED_VAR( ObjIdToGameObjMap_ );
		for ( const auto& pair : *tempObjIdToGameObjMap )
		{
			newClientProxy->GetReplicationManager()
				.ReplicateCreate( pair.first, pair.second->GetAllStateMask() );
		}
	}
}

void World::UnregistGameObj( GameObjPtr inGameObject )
{
	notifyAllClientCB_( inGameObject, RA_Destroy );

	int objId = inGameObject->GetObjId();
	auto tempFunc = [&, this]() { ObjIdToGameObjMap_->erase( objId ); };
	SET_THREAD_SHARED_VAR( ObjIdToGameObjMap_, mutex_, tempFunc );

}

void World::Update()
{
	vector< GameObjPtr > GameObjsToRem;
	auto  tempGameObjects = GET_THREAD_SHARED_VAR( ObjIdToGameObjMap_ );

	for ( const auto& pair : *tempGameObjects )
	{
		auto go = pair.second;
		if ( !( go )->DoesWantToDie() )
		{
			( go )->Update();
		}
		else
		{
			GameObjsToRem.push_back( go );
		}
	}

	if ( GameObjsToRem.size() > 0 )
	{
		for ( auto g : GameObjsToRem )
			UnregistGameObj( g );
	}
}

#else //IS_LINUX


int World::kNewObjId = 1;
World::World() {}

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
#endif //IS_LINUX
