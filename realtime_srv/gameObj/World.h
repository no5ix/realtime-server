#pragma once

#ifdef IS_LINUX
#include <muduo/base/Mutex.h>
#include <muduo/base/Atomic.h>
#endif //IS_LINUX

namespace realtime_srv {

class GameObj;

class World : noncopyable {
public:
	typedef unordered_map< int, GameObjPtr > NetIdToGameObjMap;
	typedef std::shared_ptr< unordered_map< int, GameObjPtr > > NetIdToGameObjMapPtr;
	typedef std::function<void( GameObjPtr, ReplicationAction )> NotifyAllClientCB;

	World();
	void Update();

	void SetNotifyAllClientCallback( const NotifyAllClientCB& cb ) { notifyAllClientCB_ = cb; }

	GameObjPtr GetGameObject( int inNetworkId );
	void Registry( GameObjPtr inGameObject, ReplicationAction inAction );
	void RegistGameObj( GameObjPtr inGameObject );
	void UnregistGameObj( GameObjPtr inGameObject );

private:
	int	GetNewObjId();

private:
	NotifyAllClientCB notifyAllClientCB_;

	#ifdef IS_LINUX
	muduo::MutexLock mutex_;
	static muduo::AtomicInt32		kNewObjId;
	THREAD_SHARED_VAR_DEF( private, NetIdToGameObjMap, netIdToGameObjMap_, mutex_ );

	#else //IS_LINUX

private:
	NetIdToGameObjMap			netIdToGameObjMap_;
	static int					kNewObjId;

	#endif //IS_LINUX

};
}