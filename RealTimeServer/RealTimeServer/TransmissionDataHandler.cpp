#include "RealTimeSrvPCH.h"

void TransmissionDataHandler::AddTransmission( int inNetworkId, ReplicationAction inAction, uint32_t inState )
{
	/*
		for( const auto& transmission: mTransmissions )
	{
		assert( inNetworkId != transmission.GetNetworkId() );
	}
	*/
	mTransmissions.emplace_back( inNetworkId, inAction, inState );
}

void TransmissionDataHandler::HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const
{
	for ( const ReplicationTransmission& rt : mTransmissions )
	{
		int networkId = rt.GetNetworkId();

		switch ( rt.GetAction() )
		{
		case RA_Create:
			HandleCreateDeliveryFailure( networkId );
			break;
		case RA_Update:
			HandleUpdateStateDeliveryFailure( networkId, rt.GetState(), inDeliveryNotificationManager );
			break;
		case RA_Destroy:
			HandleDestroyDeliveryFailure( networkId );
			break;
		}

	}
}

void TransmissionDataHandler::HandleCreateDeliveryFailure( int inNetworkId ) const
{
	GameObjectPtr gameObject = NetworkMgrSrv::sInst->GetGameObject( inNetworkId );
	if ( gameObject )
	{
		//LOG( "inNetworkId = %d", inNetworkId );

		mReplicationManagerServer->ReplicateCreate( inNetworkId, gameObject->GetAllStateMask() );
	}
}

void TransmissionDataHandler::HandleDestroyDeliveryFailure( int inNetworkId ) const
{
	mReplicationManagerServer->ReplicateDestroy( inNetworkId );
}

void TransmissionDataHandler::HandleUpdateStateDeliveryFailure( int inNetworkId, uint32_t inState, DeliveryNotifyMgr* inDeliveryNotificationManager ) const
{
	if ( NetworkMgrSrv::sInst->GetGameObject( inNetworkId ) )
	{
		for ( const auto& inFlightPacket : inDeliveryNotificationManager->GetInFlightPackets() )
		{
			ReplicationManagerTransmissionDataPtr rmtdp = std::static_pointer_cast< TransmissionDataHandler >( inFlightPacket.GetTransmissionData( 'RPLM' ) );
			//const TransmissionDataHandler* rmtdp = static_cast< const TransmissionDataHandler* >( inFlightPacket.GetTransmissionData( 'RPLM' ) );

			for ( const ReplicationTransmission& otherRT : rmtdp->mTransmissions )
			{
				inState &= ~otherRT.GetState();
			}
		}

		if ( inState )
		{
			mReplicationManagerServer->SetStateDirty( inNetworkId, inState );
		}
	}
}

void TransmissionDataHandler::HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const
{
	for ( const ReplicationTransmission& rt : mTransmissions )
	{
		switch ( rt.GetAction() )
		{
		case RA_Create:
			HandleCreateDeliverySuccess( rt.GetNetworkId() );
			break;
		case RA_Destroy:
			HandleDestroyDeliverySuccess( rt.GetNetworkId() );
			break;
		}
	}
}

void TransmissionDataHandler::HandleCreateDeliverySuccess( int inNetworkId ) const
{
	mReplicationManagerServer->HandleCreateAckd( inNetworkId );
}

void TransmissionDataHandler::HandleDestroyDeliverySuccess( int inNetworkId ) const
{
	mReplicationManagerServer->RemoveFromReplication( inNetworkId );
}