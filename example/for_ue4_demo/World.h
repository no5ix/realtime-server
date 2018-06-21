#pragma once

#ifdef IS_LINUX

#include <muduo/base/Mutex.h>

using namespace muduo;

typedef std::set< EntityPtr > GameObjs;
typedef std::shared_ptr< GameObjs > GameObjectsPtr;
#endif //IS_LINUX

class World
{
public:

	static void StaticInit();

	static std::unique_ptr< World >		sInst;

	void AddGameObject( EntityPtr inGameObject );
	void RemoveGameObject( EntityPtr inGameObject );

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
	const std::vector< EntityPtr >&	GetGameObjects()	const { return mGameObjects; }

private:
	std::vector< EntityPtr >	mGameObjects;

#endif //IS_LINUX

};