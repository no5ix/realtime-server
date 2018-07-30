#include "realtime_srv/game_obj/World.h"
#include "realtime_srv/rep/DeliveryNotifyMgr.h"
#include "realtime_srv/rep/ReplicationMgr.h"
#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/net/ClientProxy.h"

#include "realtime_srv/rep/InflightPacket.h"


using namespace realtime_srv;


InflightPacket::InflightPacket(
	PacketSN sequenceNum,
	ClientProxy* clientProxy) :
	SN_(sequenceNum),
	timeDispatched_(RealtimeSrvTiming::sInst.GetCurrentGameTime()),
	owner_(clientProxy)
{}

void InflightPacket::AddTransmission(int objId,
	ReplicationAction repAction, uint32_t writtenState)
{
	objIdToTransMap_.emplace(std::make_pair(
		objId,
		ReplicationTransmission(objId, repAction, writtenState)));
}

void InflightPacket::HandleDeliveryFailure() const
{
	const ReplicationTransmission *rt = nullptr;
	for (const auto& ipair : objIdToTransMap_)
	{
		rt = &ipair.second;
		int objId = rt->GetObjId();
		switch (rt->GetAction())
		{
			case RA_Create:
				HandleCreateDeliveryFailure(objId);
				break;
			case RA_Update:
				HandleUpdateStateDeliveryFailure(objId, rt->GetState());
				break;
			case RA_Destroy:
				HandleDestroyDeliveryFailure(objId);
				break;
			default:
				break;
		}
	}
}

void InflightPacket::HandleCreateDeliverySuccess(int objId) const
{ owner_->GetReplicationMgr().HandleCreateAckd(objId); }

void InflightPacket::HandleCreateDeliveryFailure(int objId) const
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject(objId);
	if (gameObject)
		owner_->GetReplicationMgr().ReplicateCreate(
			objId, gameObject->GetAllStateMask());
}

void InflightPacket::HandleUpdateStateDeliveryFailure(int objId,
	uint32_t writtenState) const
{
	if (owner_->GetWorld()->IsGameObjectExist(objId))
	{
		for (const auto& fp : owner_->GetDeliveryNotifyMgr().GetInflightPackets())
		{
			auto TransIt = fp.objIdToTransMap_.find(objId);
			if (TransIt != objIdToTransMap_.end())
				writtenState &= ~(TransIt->second.GetState());
		}
		if (writtenState)
			owner_->GetReplicationMgr().SetReplicationStateDirty(objId, writtenState);
	}
}

void InflightPacket::HandleDeliverySuccess() const
{
	const ReplicationTransmission *rt = nullptr;
	for (const auto& ipair : objIdToTransMap_)
	{
		rt = &ipair.second;
		switch (rt->GetAction())
		{
			case RA_Create:
				HandleCreateDeliverySuccess(rt->GetObjId());
				break;
			case RA_Destroy:
				HandleDestroyDeliverySuccess(rt->GetObjId());
				break;
			default:
				break;
		}
	}
}

void InflightPacket::HandleDestroyDeliveryFailure(int objId) const
{ owner_->GetReplicationMgr().ReplicateDestroy(objId); }

void InflightPacket::HandleDestroyDeliverySuccess(int objId) const
{ owner_->GetReplicationMgr().RemoveFromReplication(objId); }