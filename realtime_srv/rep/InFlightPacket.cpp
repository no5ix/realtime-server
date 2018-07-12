#include "realtime_srv/common/RealtimeSrvShared.h"

using namespace realtime_srv;


InFlightPacket::InFlightPacket(
	PacketSN inSequenceNumber,
	ClientProxy* inClientProxy ) :
	mSequenceNumber( inSequenceNumber ),
	mTimeDispatched( RealtimeSrvTiming::sInst.GetCurrentGameTime() ),
	owner_( inClientProxy )
{}

void InFlightPacket::AddTransmission( int inObjId,
	ReplicationAction inAction, uint32_t inState )
{
	NetIdToTransMap_.emplace( std::make_pair(
		inObjId,
		ReplicationTransmission( inObjId, inAction, inState ) ) );
}

void InFlightPacket::HandleDeliveryFailure() const
{
	const ReplicationTransmission *rt = nullptr;
	for ( const auto& ipair : NetIdToTransMap_ )
	{
		rt = &ipair.second;
		int networkId = rt->GetObjId();

		switch ( rt->GetAction() )
		{
			case RA_Create:
				HandleCreateDeliveryFailure( networkId );
				break;
			case RA_Update:
				HandleUpdateStateDeliveryFailure( networkId, rt->GetState() );
				break;
			case RA_Destroy:
				HandleDestroyDeliveryFailure( networkId );
				break;
			default:
				break;
		}
	}
}

void InFlightPacket::HandleCreateDeliveryFailure( int inObjId ) const
{
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inObjId );
	if ( gameObject )
	{
		owner_->GetReplicationManager().ReplicateCreate(
			inObjId, gameObject->GetAllStateMask() );
	}
}

void InFlightPacket::HandleDestroyDeliveryFailure( int inObjId ) const
{
	owner_->GetReplicationManager().ReplicateDestroy( inObjId );
}

void realtime_srv::InFlightPacket::HandleUpdateStateDeliveryFailure( int inObjId,
	uint32_t inState ) const
{
	if ( owner_->GetWorld()->IsGameObjectExist( inObjId ) )
	{
		for ( const auto& iFlightPacket : owner_->GetDeliveryNotifyManager().GetInFlightPackets() )
		{
			auto TransIt = iFlightPacket.NetIdToTransMap_.find( inObjId );
			if (TransIt != NetIdToTransMap_.end())
			{
				inState &= ~( TransIt->second.GetState() );
			}
		}

		if ( inState )
		{
			owner_->GetReplicationManager().SetReplicationStateDirty( inObjId, inState );
		}
	}
}

void realtime_srv::InFlightPacket::HandleDeliverySuccess() const
{
	const ReplicationTransmission *rt = nullptr;
	for ( const auto& ipair : NetIdToTransMap_ )
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

void InFlightPacket::HandleCreateDeliverySuccess( int inObjId ) const
{
	owner_->GetReplicationManager().HandleCreateAckd( inObjId );
}

void InFlightPacket::HandleDestroyDeliverySuccess( int inObjId ) const
{
	owner_->GetReplicationManager().RemoveFromReplication( inObjId );
}