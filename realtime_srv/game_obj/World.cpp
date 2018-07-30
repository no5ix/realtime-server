#include "realtime_srv/common/RealtimeSrvHelper.h"
#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/net/ClientProxy.h"

#include "realtime_srv/game_obj/World.h"


using namespace realtime_srv;



int World::kNewObjId = 1;

void World::Registry(std::shared_ptr<GameObj> object, ReplicationAction repAction)
{
	if (repAction == RA_Create)
		return RegistGameObj(object);
	else if (repAction == RA_Destroy)
		return UnregistGameObj(object);
}

void World::RegistGameObj(std::shared_ptr<GameObj> object)
{
	int newObjId = GetNewObjId();
	object->SetObjId(newObjId);
	object->SetWorld(shared_from_this());
	ObjIdToGameObjMap_[newObjId] = object;

	onObjCreateOrDestoryCb_(object, RA_Create);
}

void World::UnregistGameObj(std::shared_ptr<GameObj> object)
{
	onObjCreateOrDestoryCb_(object, RA_Destroy);
	ObjIdToGameObjMap_.erase(object->GetObjId());
}

void World::WhenClientProxyHere(std::shared_ptr<ClientProxy> clientProxy)
{
	if (clientProxy)
	{
		clientProxy->SetWorld(shared_from_this());
		for (const auto& ipair : ObjIdToGameObjMap_)
		{
			clientProxy->GetReplicationMgr().ReplicateCreate(
				ipair.first, ipair.second->GetAllStateMask());
		}
	}
}

int World::GetNewObjId()
{
	int toRet = kNewObjId++;
	if (kNewObjId < toRet)
		LOG("GameObj ID Wrap Around!!! You've been playing way too long...");
	return toRet;
}

bool World::IsGameObjectExist(int objectId)
{
	auto gameObjectIt = ObjIdToGameObjMap_.find(objectId);
	return gameObjectIt != ObjIdToGameObjMap_.end();
}

GameObjPtr World::GetGameObject(int objectId)
{
	auto gameObjectIt = ObjIdToGameObjMap_.find(objectId);
	if (gameObjectIt != ObjIdToGameObjMap_.end())
		return gameObjectIt->second;
	else
		return GameObjPtr();
}

void World::Update()
{
	for (auto it = ObjIdToGameObjMap_.begin();
		it != ObjIdToGameObjMap_.end(); )
	{
		if (it->second->IsPendingToDie())
			UnregistGameObj(it++->second);
		else
			it++->second->Update();
	}
}
