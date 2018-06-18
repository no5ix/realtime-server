#pragma once

#ifdef NEW_EPOLL_INTERFACE

#include <muduo/base/Mutex.h>

using namespace muduo;

typedef std::set< EntityPtr > GameObjs;
typedef std::shared_ptr< GameObjs > GameObjectsPtr;
#endif //NEW_EPOLL_INTERFACE

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

#ifdef NEW_EPOLL_INTERFACE

public:
	GameObjectsPtr GetGameObjects();
	void GameObjectsCOW();

private:
	GameObjectsPtr mGameObjects;
	MutexLock mutex_;

#else //NEW_EPOLL_INTERFACE

public:
	const std::vector< EntityPtr >&	GetGameObjects()	const { return mGameObjects; }

private:
	std::vector< EntityPtr >	mGameObjects;

#endif //NEW_EPOLL_INTERFACE

};