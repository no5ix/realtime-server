#pragma once

#include <unordered_map>
#include "realtime_srv/common/RealtimeSrvMacro.h"
#include "realtime_srv/rep/ReplicationCmd.h"


namespace realtime_srv
{


class DeliveryNotifyMgr;
class ReplicationMgr;
class ClientProxy;


class InflightPacket
{
public:
	InflightPacket(PacketSN sequenceNum, ClientProxy* clientProxy);

	PacketSN GetSequenceNumber() const { return SN_; }
	float GetTimeDispatched() const { return timeDispatched_; }
public:

	class ReplicationTransmission
	{
	public:
		ReplicationTransmission(int objId,
			ReplicationAction repAction, uint32_t writtenState)
			:
			ObjId_(objId),
			repAction_(repAction),
			writtenState_(writtenState)
		{}

		int								GetObjId()		const { return ObjId_; }
		ReplicationAction	GetAction()			const { return repAction_; }
		uint32_t					GetState()			const { return writtenState_; }

	private:
		int							ObjId_;
		ReplicationAction			repAction_;
		uint32_t					writtenState_;
	};

	void AddTransmission(int objId, ReplicationAction repAction, uint32_t writtenState);

	virtual void HandleDeliveryFailure() const;
	virtual void HandleDeliverySuccess() const;


private:

	void HandleCreateDeliveryFailure(int objId) const;
	void HandleUpdateStateDeliveryFailure(int objId,
		uint32_t writtenState) const;

	void HandleDestroyDeliveryFailure(int objId) const;

	void HandleCreateDeliverySuccess(int objId) const;

	void HandleDestroyDeliverySuccess(int objId) const;


private:
	std::unordered_map< int, ReplicationTransmission >	objIdToTransMap_;

	PacketSN	SN_;
	float		timeDispatched_;
	ClientProxy* owner_;
};

}