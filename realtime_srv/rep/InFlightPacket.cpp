#include "realtime_srv/common/RealtimeSrvShared.h"

using namespace realtime_srv;


InFlightPacket::InFlightPacket(
	PacketSN inSequenceNumber,
	ClientProxy* inClientProxy ) :
	mSequenceNumber( inSequenceNumber ),
	mTimeDispatched( RealtimeSrvTiming::sInstance.GetCurrentGameTime() ),
	owner_( inClientProxy ) {}

void InFlightPacket::AddTransmission( int inNetworkId, ReplicationAction inAction, uint32_t inState ) {
	mTransmissions.emplace_back( inNetworkId, inAction, inState );
}

void InFlightPacket::HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const {
	for ( const ReplicationTransmission& rt : mTransmissions ) {
		int networkId = rt.GetNetworkId();

		switch ( rt.GetAction() ) {
			case RA_Create:
				HandleCreateDeliveryFailure( networkId );
				break;
			case RA_Update:
				HandleUpdateStateDeliveryFailure( networkId, rt.GetState(), inDeliveryNotificationManager );
				break;
			case RA_Destroy:
				HandleDestroyDeliveryFailure( networkId );
				break;
			default:
				break;
		}

	}
}

void InFlightPacket::HandleCreateDeliveryFailure( int inNetworkId ) const {
	GameObjPtr gameObject = owner_->GetWorld()->GetGameObject( inNetworkId );
	if ( gameObject ) {
		owner_->GetReplicationManager().ReplicateCreate( inNetworkId, gameObject->GetAllStateMask() );
	}
}

void InFlightPacket::HandleDestroyDeliveryFailure( int inNetworkId ) const {
	owner_->GetReplicationManager().ReplicateDestroy( inNetworkId );
}

void InFlightPacket::HandleUpdateStateDeliveryFailure( int inNetworkId, uint32_t inState, DeliveryNotifyMgr* inDeliveryNotificationManager ) const {
	if ( owner_->GetWorld()->GetGameObject( inNetworkId ) ) {
		for ( const auto& inFlightPacket : inDeliveryNotificationManager->GetInFlightPackets() ) {
			for ( const ReplicationTransmission& otherRT : inFlightPacket.mTransmissions ) {
				inState &= ~otherRT.GetState();
			}
		}

		if ( inState ) {
			owner_->GetReplicationManager().SetReplicationStateDirty( inNetworkId, inState );
		}
	}
}

void InFlightPacket::HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const {
	for ( const ReplicationTransmission& rt : mTransmissions ) {
		switch ( rt.GetAction() ) {
			case RA_Create:
				HandleCreateDeliverySuccess( rt.GetNetworkId() );
				break;
			case RA_Destroy:
				HandleDestroyDeliverySuccess( rt.GetNetworkId() );
				break;
			default:
				break;
		}
	}
}

void InFlightPacket::HandleCreateDeliverySuccess( int inNetworkId ) const {
	owner_->GetReplicationManager().HandleCreateAckd( inNetworkId );
}

void InFlightPacket::HandleDestroyDeliverySuccess( int inNetworkId ) const {
	owner_->GetReplicationManager().RemoveFromReplication( inNetworkId );
}