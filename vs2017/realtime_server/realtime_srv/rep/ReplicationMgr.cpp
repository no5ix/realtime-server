
#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/game_obj/World.h"
#include "realtime_srv/rep/InflightPacket.h"
#include "realtime_srv/net/ClientProxy.h"

#include "realtime_srv/rep/ReplicationMgr.h"


using namespace realtime_srv;


void ReplicationMgr::Write(OutputBitStream& outputStream, InflightPacket* inflightPkt)
{
	for (auto& pair : objIdToRepCmd_)
	{
		ReplicationCmd& repCmd = pair.second;
		if (repCmd.HasDirtyState())
		{
			int objId = pair.first;
			outputStream.Write(objId);

			ReplicationAction repAction = repCmd.GetAction();
			outputStream.Write(repAction, 2);

			uint32_t dirtyState = repCmd.GetDirtyState();

			uint32_t writtenRepState = 0;
			switch (repAction)
			{
				case RA_Create:
					writtenRepState = WriteCreateAction(outputStream,
						objId, dirtyState);
					break;
				case RA_Update:
					writtenRepState = WriteUpdateAction(outputStream,
						objId, dirtyState);
					break;
				case RA_Destroy:
					writtenRepState = WriteDestroyAction(outputStream,
						objId, dirtyState);
					break;
				default:
					break;
			}

			inflightPkt->AddTransmission(objId, repAction, writtenRepState);
			repCmd.ClearDirtyState(writtenRepState);
			if (outputStream.GetByteLength() > MAX_PACKET_BYTE_LENGTH)
				break;
		}
	}
}

uint32_t ReplicationMgr::WriteCreateAction(OutputBitStream& outputStream,
	int objId, uint32_t dirtyState)
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject(objId);
	assert(gameObject);
	outputStream.Write(gameObject->GetClassId());
	return gameObject->Write(outputStream, dirtyState);
}

uint32_t ReplicationMgr::WriteUpdateAction(OutputBitStream& outputStream,
	int objId, uint32_t dirtyState)
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject(objId);
	assert(gameObject);
	return gameObject->Write(outputStream, dirtyState);
}

uint32_t ReplicationMgr::WriteDestroyAction(OutputBitStream& outputStream,
	int objId, uint32_t dirtyState)
{
	(void)outputStream;
	(void)objId;
	(void)dirtyState;

	return dirtyState;
}