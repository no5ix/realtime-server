#pragma once

#include <unordered_map>
#include <memory>

#include <muduo/base/Logging.h>
#include <muduo_udp_support/UdpConnection.h>

#include "realtime_srv/common/RealtimeSrvTiming.h"
#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/game_obj/ActionList.h"
#include "realtime_srv/rep/DeliveryNotifyMgr.h"
#include "realtime_srv/rep/ReplicationMgr.h"

namespace realtime_srv
{

class NetworkMgr;
class World;

class ClientProxy
{
public:
	typedef std::unordered_map<int, std::shared_ptr<GameObj>> ObjIdToGameObjMap;

	int	GetNetId() const { return netId_; }

	void AddGameObj( const std::shared_ptr<GameObj>& _gameObj )
	{ objIdToGameObjMap_[_gameObj->GetObjId()] = _gameObj; }

	ObjIdToGameObjMap& GetAllOwnedGameObjs() { return objIdToGameObjMap_; }
	const ObjIdToGameObjMap& GetAllOwnedGameObjs() const { return objIdToGameObjMap_; }

	void SetAllOwnedGameObjsPendingToDie();
	void RealeaseAllOwnedGameObjs();

	void RealeaseSpecificOwnedGameObj( int _objId )
	{ objIdToGameObjMap_.erase( _objId ); }

	void UpdateLastPacketTime()
	{ mLastPacketFromClientTime = RealtimeSrvTiming::sInst.GetCurrentGameTime(); }

	float GetLastPacketFromClientTime()	const { return mLastPacketFromClientTime; }

	DeliveryNotifyMgr&	GetDeliveryNotifyMgr() { return deliveryNotifyMgr_; }
	ReplicationMgr&	GetReplicationMgr() { return replicationMgr_; }

	std::shared_ptr<World>& GetWorld() { return world_; }
	void SetWorld( std::shared_ptr<World>  world ) { world_ = world; }

	std::shared_ptr<NetworkMgr>& GetNetworkManager() { return networkManager_; }

	const ActionList& GetUnprocessedActionList() const { return mUnprocessedMoveList; }
	ActionList&	 GetUnprocessedActionList() { return mUnprocessedMoveList; }

	void SetIsLastMoveTimestampDirty( bool inIsDirty )
	{ mIsLastMoveTimestampDirty = inIsDirty; }
	bool IsLastMoveTimestampDirty()	const
	{ return mIsLastMoveTimestampDirty; }

	bool GetRecvingServerResetFlag() const
	{ return mRecvingServerResetFlag; }
	void SetRecvingServerResetFlag( bool inRecvingServerResetFlag )
	{ mRecvingServerResetFlag = inRecvingServerResetFlag; }


private:

	int				netId_;

	ObjIdToGameObjMap objIdToGameObjMap_;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;

	bool			mRecvingServerResetFlag;

	DeliveryNotifyMgr		deliveryNotifyMgr_;
	ReplicationMgr			replicationMgr_;
	std::shared_ptr<World>			world_;
	std::shared_ptr<NetworkMgr>		networkManager_;


public:
	ClientProxy( const std::shared_ptr<NetworkMgr>& inNetworkManager,
		const int inNetId,
		const pid_t inHoldedByThreadId,
		const muduo::net::UdpConnectionPtr& inUdpConnection );

	~ClientProxy() { LOG_INFO << "Netid " << netId_ << " disconnect"; }

	pid_t GetConnHoldedByThreadId() const { return connHoldedByTid_; }
	muduo::net::UdpConnectionPtr& GetUdpConnection() { return UdpConnection_; }

private:
	muduo::net::UdpConnectionPtr	UdpConnection_;
	pid_t													connHoldedByTid_;
};

typedef std::shared_ptr<ClientProxy>	ClientProxyPtr;
typedef std::weak_ptr<ClientProxy>	ClientProxyWPtr;

}