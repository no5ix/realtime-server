#include "realtime_srv/rep/DeliveryNotifyMgr.h"
#include "realtime_srv/rep/ReplicationMgr.h"
#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/net/ClientProxy.h"

#include "realtime_srv/rep/InflightPacket.h"


using namespace realtime_srv;


InflightPacket::InflightPacket(
	PacketSN inSequenceNumber,
	ClientProxy* inClientProxy ) :
	mSequenceNumber( inSequenceNumber ),
	mTimeDispatched( RealtimeSrvTiming::sInst.GetCurrentGameTime() ),
	owner_( inClientProxy )
{}

void InflightPacket::AddTransmission( int inObjId,
	ReplicationAction inAction, uint32_t inState )
{
	objIdToTransMap_.emplace( std::make_pair(
		inObjId,
		ReplicationTransmission( inObjId, inAction, inState ) ) );
}

void InflightPacket::HandleDeliveryFailure() const
{
	const ReplicationTransmission *rt = nullptr;
	for ( const auto& ipair : objIdToTransMap_ )
	{
		rt = &ipair.second;
		int objId = rt->GetObjId();
		switch ( rt->GetAction() )
		{
			case RA_Create:
				HandleCreateDeliveryFailure( objId );
				break;
			case RA_Update:
				HandleUpdateStateDeliveryFailure( objId, rt->GetState() );
				break;
			case RA_Destroy:
				HandleDestroyDeliveryFailure( objId );
				break;
			default:
				break;
		}
	}
}

void InflightPacket::HandleCreateDeliveryFailure( int inObjId ) const
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inObjId );
	if ( gameObject )
		owner_->GetReplicationMgr().ReplicateCreate(
			inObjId, gameObject->GetAllStateMask() );
}

void InflightPacket::HandleUpdateStateDeliveryFailure( int inObjId,
	uint32_t inState ) const
{
	if ( owner_->GetWorld()->IsGameObjectExist( inObjId ) )
	{
		for ( const auto& fp : owner_->GetDeliveryNotifyMgr().GetInflightPackets() )
		{
			auto TransIt = fp.objIdToTransMap_.find( inObjId );
			if (TransIt != objIdToTransMap_.end())
				inState &= ~( TransIt->second.GetState() );
		}
		if ( inState )
			owner_->GetReplicationMgr().SetReplicationStateDirty( inObjId, inState );
	}
}

void InflightPacket::HandleDeliverySuccess() const
{
	const ReplicationTransmission *rt = nullptr;
	for ( const auto& ipair : objIdToTransMap_ )
	{
		rt = &ipair.second;
		switch ( rt->GetAction() )
		{
			case RA_Create:
				HandleCreateDeliverySuccess( rt->GetObjId() );
				break;
			case RA_Destroy:
				HandleDestroyDeliverySuccess( rt->GetObjId() );
				break;
			default:
				break;
		}
	}
}