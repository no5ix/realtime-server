#pragma once

#ifdef IS_LINUX

#include <muduo/base/Mutex.h>

using namespace muduo;

typedef std::set< GameObjPtr > GameObjs;
typedef std::shared_ptr< GameObjs > GameObjectsPtr;
#endif //IS_LINUX

class World
{
public:

	static void StaticInit();

	static std::unique_ptr< World >		sInst;

	void AddGameObject( GameObjPtr inGameObject );
	void RemoveGameObject( GameObjPtr inGameObject );

	void Update();

private:
	World();
	//int	GetIndexOfGameObject( GameObjectPtr inGameObject );

#ifdef IS_LINUX

public:
	GameObjectsPtr GetGameObjects();
	void GameObjectsCOW();

private:
	GameObjectsPtr mGameObjects;
	MutexLock mutex_;

#else //IS_LINUX

public:
	const std::vector< GameObjPtr >&	GetGameObjects()	const { return mGameObjects; }

private:
	std::vector< GameObjPtr >	mGameObjects;

#endif //IS_LINUX

};