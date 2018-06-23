#pragma once

#ifdef IS_LINUX

#include <muduo/base/Mutex.h>

using namespace muduo;
#endif //IS_LINUX

class World
{
public:

	static void StaticInit();

	static std::unique_ptr< World >		sInstance;

	void AddGameObject( GameObjPtr inGameObject );
	void RemoveGameObject( GameObjPtr inGameObject );

	void Update();

private:
	World();

#ifdef IS_LINUX

	typedef std::set< GameObjPtr > GameObjs;
	typedef std::shared_ptr< GameObjs > GameObjectsPtr;
public:
	//GameObjectsPtr GetGameObjects();
	//void GameObjectsCOW();

private:
	//GameObjectsPtr GameObjects_;
	THREAD_SHARED_VAR_DEF( private, GameObjs, GameObjects_, mutex_ );
	MutexLock mutex_;

#else //IS_LINUX

public:
	const std::vector< GameObjPtr >& GetGameObjects() const 
	{ return GameObjects_; }

private:
	std::vector< GameObjPtr >	GameObjects_;

#endif //IS_LINUX

};