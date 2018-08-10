#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/net/ClientProxy.h"
#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/common/any.h"

#include "realtime_srv/net/NetworkMgr.h"


using namespace realtime_srv;


//#ifdef IS_LINUX


using namespace muduo;
using namespace muduo::net;

AtomicInt32 NetworkMgr::kNewNetId;

NetworkMgr::NetworkMgr(const ServerConfig serverConfig) :
	bUnregistObjWhenCliDisconn_(serverConfig.is_unregist_obj_when_cli_disconn),
	clientDisconnTimeout_(serverConfig.client_disconnect_timeout),
	actionCountPerTick_(serverConfig.action_count_per_tick),
	pktHandler_(serverConfig,
		std::bind(&NetworkMgr::DoProcessPkt, this, _1),
		std::bind(&NetworkMgr::Tick, this))
{
	assert(actionCountPerTick_ >= 1);
	assert(clientDisconnTimeout_ > 0);

	kNewNetId.getAndSet(1);

	pktHandler_.SetInterval(std::bind(&NetworkMgr::CheckForDisconnects, this),
		clientDisconnTimeout_);
}

void NetworkMgr::UpdateConnListForCheckDisconn(const UdpConnectionPtr& conn,
	UpdateConnListFlag flag, const Timestamp& time)
{
	switch (flag)
	{
		case NetworkMgr::INSERT:
		{
			if (conn->connected())
			{
				NodeForCheckDisconn node;
				node.lastRecvTime = Timestamp::now();
				connListForCheckDisconn_.push_back(conn);
				node.position = --connListForCheckDisconn_.end();
				conn->setContext(node);
			}
			break;
		}
		case NetworkMgr::DELETE:
		{
			if (!conn->connected())
			{
				assert(!conn->getContext().empty());
				const NodeForCheckDisconn& node =
					realtime_srv::any_cast<const NodeForCheckDisconn&>(conn->getContext());
				connListForCheckDisconn_.erase(node.position);
			}
			break;
		}
		case NetworkMgr::UPDATE:
		{
			if (conn->connected())
			{
				assert(!conn->getContext().empty());
				assert(time.valid());
				NodeForCheckDisconn* node =
					realtime_srv::any_cast<NodeForCheckDisconn>(conn->getMutableContext());
				node->lastRecvTime = time;
				connListForCheckDisconn_.splice(
					connListForCheckDisconn_.end(), connListForCheckDisconn_, node->position);
				assert(node->position == --connListForCheckDisconn_.end());
			}
			break;
		}
		default:
			break;
	}
}

void NetworkMgr::Start()
{
	//assert( newPlayerCb_ );
	//assert( customInputStatecb_ );

	assert(letCliProxyGetWorldState_);
	assert(worldRegistryCb_);
	assert(worldUpdateCb_);

	pktHandler_.Start();
}

void NetworkMgr::DoPreparePacketToSend(std::shared_ptr<ClientProxy>& clientProxy,
	const uint32_t connectionFlag)
{
	OutputBitStreamPtr outputPacket(new OutputBitStream());
	outputPacket->Write(connectionFlag);

	InflightPacket* ifp = clientProxy->GetDeliveryNotifyMgr()
		.WriteState(*outputPacket, clientProxy.get());

	switch (connectionFlag)
	{
		case kResetCC:
			clientProxy->SetRecvingServerResetFlag(true);
			LOG_INFO << "Send Reset Packet";
		case kWelcomeCC:
			outputPacket->Write(clientProxy->GetNetId());
			outputPacket->Write(pktHandler_.GetSendPacketInterval());
			break;
		case kStateCC:
			WriteLastMoveTimestampIfDirty(*outputPacket, clientProxy);
		default:
			break;
	}
	clientProxy->GetReplicationMgr().Write(*outputPacket, ifp);

	pktHandler_.AddToPendingSndPktQ(
		PendingSendPacketPtr(new PendingSendPacket(
			outputPacket, clientProxy->GetUdpConnection())),
		clientProxy->GetConnHoldedByThreadId());
}

void NetworkMgr::DoProcessPkt(ReceivedPacketPtr& recvedPacket)
{
	auto it = udpConnToClientMap_.find(recvedPacket->GetUdpConn());
	if (it == udpConnToClientMap_.end())
	{
		WelcomeNewClient(*recvedPacket->GetPacketBuffer(),
			recvedPacket->GetUdpConn(), recvedPacket->GetHoldedByThreadId());
	}
	else
	{
		UpdateConnListForCheckDisconn(recvedPacket->GetUdpConn(),
			UpdateConnListFlag::UPDATE, recvedPacket->GetReceivedTime());
		CheckPacketType((*it).second, *recvedPacket->GetPacketBuffer());
	}
}

void NetworkMgr::PreparePacketToSend()
{
	for (auto& pair : udpConnToClientMap_)
	{
		(pair.second)->GetDeliveryNotifyMgr().ProcessTimedOutPackets();
		if ((pair.second)->IsLastMoveTimestampDirty())
			DoPreparePacketToSend((pair.second), kStateCC);
	}
}

void NetworkMgr::Tick()
{
	worldUpdateCb_();
	PreparePacketToSend();
}

void NetworkMgr::CheckForDisconnects()
{
	Timestamp now = Timestamp::now();
	for (WeakConnectionList::iterator it = connListForCheckDisconn_.begin();
		it != connListForCheckDisconn_.end();)
	{
		UdpConnectionPtr conn = it->lock();
		if (conn)
		{
			++it;
			NodeForCheckDisconn* curNode =
				realtime_srv::any_cast<NodeForCheckDisconn>(conn->getMutableContext());
			double age = timeDifference(now, curNode->lastRecvTime);
			if (age > clientDisconnTimeout_)
			{
				if (conn->connected())
				{
					if (bUnregistObjWhenCliDisconn_)
						udpConnToClientMap_.at(conn)->SetAllOwnedGameObjsPendingToDie();
					else
						udpConnToClientMap_.at(conn)->RealeaseAllOwnedGameObjs();

					conn->forceClose();
					UpdateConnListForCheckDisconn(conn, UpdateConnListFlag::DELETE);
					udpConnToClientMap_.erase(conn);
				}
			}
			else if (age < 0)
			{
				LOG_WARN << "Time jump";
				curNode->lastRecvTime = now;
			}
			else
				break;
		}
		else
		{
			LOG_WARN << "Expired";
			it = connListForCheckDisconn_.erase(it);
		}
	}
}

ClientProxyPtr NetworkMgr::CreateNewClient(
	UdpConnectionPtr& udpConnetction, const pid_t holdedByThreadId)
{
	ClientProxyPtr newClientProxy = std::make_shared<ClientProxy>(
		shared_from_this(),
		udpConnetction->GetConnId(),
		holdedByThreadId,
		udpConnetction);

	udpConnToClientMap_[udpConnetction] = newClientProxy;
	UpdateConnListForCheckDisconn(udpConnetction, UpdateConnListFlag::INSERT);

	letCliProxyGetWorldState_(newClientProxy);

	if (newPlayerCb_)
	{
		GameObjPtr newControlledGameObj(newPlayerCb_(newClientProxy));
		if (newControlledGameObj)
		{
			worldRegistryCb_(newControlledGameObj, RA_Create);
			newControlledGameObj->SetMaster(newClientProxy);
		}
	}

	return newClientProxy;
}

void NetworkMgr::WelcomeNewClient(InputBitStream& inputStream,
	UdpConnectionPtr& udpConnetction, const pid_t holdedByThreadId)
{
	uint32_t	packetType;
	inputStream.Read(packetType);

	switch (packetType)
	{
		case kHelloCC:
		case kInputCC:
		case kResetedCC:
		{
			ClientProxyPtr newClientProxy =
				CreateNewClient(udpConnetction, holdedByThreadId);
			if (packetType == kHelloCC)
				DoPreparePacketToSend(newClientProxy, kWelcomeCC);
			else
				DoPreparePacketToSend(newClientProxy, kResetCC);
			LOG_INFO << "a new client as Net ID " << newClientProxy->GetNetId();
			break;
		}
		default:
			LOG_INFO << "Bad incoming packet from unknown client at socket "
				<< udpConnetction->peerAddress().toIpPort() << " is "
				<< " - we're under attack!!";
			break;
	}
}

void NetworkMgr::OnObjCreateOrDestory(GameObjPtr& gameObject, ReplicationAction repAction)
{
	if (repAction == RA_Create)
		gameObject->SetNetworkMgr(shared_from_this());

	for (const auto& pair : udpConnToClientMap_)
	{
		switch (repAction)
		{
			case RA_Create:
				pair.second->GetReplicationMgr().ReplicateCreate(
					gameObject->GetObjId(), gameObject->GetAllStateMask());
				break;
			case RA_Destroy:
				pair.second->GetReplicationMgr().ReplicateDestroy(
					gameObject->GetObjId());
				break;
			default:
				break;
		}
	}
}

void NetworkMgr::SetRepStateDirty(int objectId, uint32_t dirtyState)
{
	for (const auto& pair : udpConnToClientMap_)
		pair.second->GetReplicationMgr().SetReplicationStateDirty(
			objectId, dirtyState);
}

void NetworkMgr::CheckPacketType(std::shared_ptr<ClientProxy>& clientProxy,
	InputBitStream& inputStream)
{
	uint32_t packetType = HandleServerReset(clientProxy, inputStream);
	switch (packetType)
	{
		case kHelloCC:
			DoPreparePacketToSend(clientProxy, kWelcomeCC);
			break;
		case kInputCC:
		{
			if (clientProxy->GetDeliveryNotifyMgr().ReadAndProcessState(inputStream))
				HandleInputPacket(clientProxy, inputStream);
			break;
		}
		case kNullCC:
			break;
		default:
			LOG_INFO << "Unknown packet type received from "
				<< clientProxy->GetUdpConnection()->peerAddress().toIpPort();
			break;
	}
}

uint32_t NetworkMgr::HandleServerReset(std::shared_ptr<ClientProxy>& clientProxy,
	InputBitStream& inputStream)
{
	uint32_t packetType;
	inputStream.Read(packetType);

	if (packetType == kResetedCC)
	{
		clientProxy->SetRecvingServerResetFlag(false);
		inputStream.Read(packetType);
	}
	if (clientProxy->GetRecvingServerResetFlag() == true)
	{
		DoPreparePacketToSend(clientProxy, kWelcomeCC);
		return kNullCC;
	}
	else
		return packetType;
}

void NetworkMgr::HandleInputPacket(std::shared_ptr<ClientProxy>& clientProxy,
	InputBitStream& inputStream)
{
	uint32_t actionCount = 0;
	Action action(customInputStatecb_ ? customInputStatecb_() : (new InputState));
	inputStream.Read(actionCount, actionCountPerTick_);

	for (; actionCount > 0; --actionCount)
		if (action.Read(inputStream))
			if (clientProxy->GetUnprocessedActionList().AddMoveIfNew(action))
				clientProxy->SetIsLastMoveTimestampDirty(true);
}

void NetworkMgr::WriteLastMoveTimestampIfDirty(OutputBitStream& outputStream,
	std::shared_ptr<ClientProxy>& clientProxy)
{
	bool isTimestampDirty = clientProxy->IsLastMoveTimestampDirty();
	outputStream.Write(isTimestampDirty);
	if (isTimestampDirty)
	{
		outputStream.Write(clientProxy->GetUnprocessedActionList()
			.GetLastMoveTimestamp());
		clientProxy->SetIsLastMoveTimestampDirty(false);
	}
}


//#else
//#include "realtime_srv/net/NetworkMgrWinS.h"
//#endif