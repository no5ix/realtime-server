#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;


void World::Registry( GameObjPtr inGameObject, ReplicationAction inAction ) {
	if ( inAction == RA_Create ) {
		RegistGameObj( inGameObject );
	} else if ( inAction == RA_Destroy ) {
		UnregistGameObj( inGameObject );
	}
}


#ifdef IS_LINUX


using namespace muduo;

AtomicInt32 World::kNewObjId;

World::World() :
	netIdToGameObjMap_( new NetIdToGameObjMap ) {
	kNewObjId.getAndSet( 1 );
}

GameObjPtr World::GetGameObject( int inNetworkId ) {
	auto tempNetworkIdToGameObjectMap = GET_THREAD_SHARED_VAR( netIdToGameObjMap_ );
	auto gameObjectIt = tempNetworkIdToGameObjectMap->find( inNetworkId );
	if ( gameObjectIt != tempNetworkIdToGameObjectMap->end() ) {
		return gameObjectIt->second;
	} else {
		return GameObjPtr();
	}
}

int World::GetNewObjId() {
	int toRet = kNewObjId.getAndAdd( 1 );
	if ( kNewObjId.get() < toRet ) {
		LOG( "Network ID Wrap Around!!! You've been playing way too long..." );
	}

	return toRet;
}

void World::RegistGameObj( GameObjPtr inGameObject ) {
	int newNetworkId = GetNewObjId();
	inGameObject->SetObjId( newNetworkId );

	auto regFunc = [newNetworkId, &inGameObject, this]() { ( *netIdToGameObjMap_ )[newNetworkId] = inGameObject; };
	SET_THREAD_SHARED_VAR( netIdToGameObjMap_, mutex_, regFunc );

	notifyAllClientCB_( inGameObject, RA_Create );
	auto newClientProxy = inGameObject->GetClientProxy();
	if ( newClientProxy ) {
		newClientProxy->SetWorld( this );
		NetIdToGameObjMapPtr tempNetworkIdToGameObjectMap =
			GET_THREAD_SHARED_VAR( netIdToGameObjMap_ );
		for ( const auto& pair : *tempNetworkIdToGameObjectMap ) {
			newClientProxy->GetReplicationManager()
				.ReplicateCreate( pair.first, pair.second->GetAllStateMask() );
		}
	}
}

void World::UnregistGameObj( GameObjPtr inGameObject ) {
	notifyAllClientCB_( inGameObject, RA_Destroy );

	int networkId = inGameObject->GetObjId();
	auto tempFunc = [&, this]() { netIdToGameObjMap_->erase( networkId ); };
	SET_THREAD_SHARED_VAR( netIdToGameObjMap_, mutex_, tempFunc );

}

void World::Update() {
	vector< GameObjPtr > GameObjsToRem;
	auto  tempGameObjects = GET_THREAD_SHARED_VAR( netIdToGameObjMap_ );

	for ( const auto& pair : *tempGameObjects ) {
		auto go = pair.second;
		if ( !( go )->DoesWantToDie() ) {
			( go )->Update();
		} else {
			GameObjsToRem.push_back( go );
		}
	}

	if ( GameObjsToRem.size() > 0 ) {
		for ( auto g : GameObjsToRem )
			UnregistGameObj( g );
	}
}

#else //IS_LINUX


int World::kNewObjId = 1;
World::World() {}

int World::GetNewObjId() {
	int toRet = kNewObjId++;
	if ( kNewObjId < toRet ) {
		LOG( "Network ID Wrap Around!!! You've been playing way too long..." );
	}
	return toRet;
}

GameObjPtr World::GetGameObject( int inNetworkId ) {
	auto gameObjectIt = netIdToGameObjMap_.find( inNetworkId );
	if ( gameObjectIt != netIdToGameObjMap_.end() ) {
		return gameObjectIt->second;
	} else {
		return GameObjPtr();
	}
}

void World::RegistGameObj( GameObjPtr inGameObject ) {
	int newNetworkId = GetNewObjId();
	inGameObject->SetObjId( newNetworkId );
	netIdToGameObjMap_[newNetworkId] = inGameObject;

	notifyAllClientCB_( inGameObject, RA_Create );
	auto newClientProxy = inGameObject->GetClientProxy();
	if ( newClientProxy ) {
		newClientProxy->SetWorld( this );
		for ( const auto& pair : netIdToGameObjMap_ ) {
			newClientProxy->GetReplicationManager().ReplicateCreate(
				pair.first, pair.second->GetAllStateMask() );
		}
	}
}

void World::UnregistGameObj( GameObjPtr inGameObject ) {
	notifyAllClientCB_( inGameObject, RA_Destroy );

	int networkId = inGameObject->GetObjId();
	netIdToGameObjMap_.erase( networkId );
}

void World::Update() {
	vector< GameObjPtr > GameObjsToRem;
	for ( const auto& pair : netIdToGameObjMap_ ) {
		auto go = pair.second;

		if ( !go->DoesWantToDie() ) {
			go->Update();
		} else {
			GameObjsToRem.push_back( go );
		}
	}

	if ( GameObjsToRem.size() > 0 ) {
		for ( auto& g : GameObjsToRem )
			UnregistGameObj( g );
	}
}
#endif //IS_LINUX
