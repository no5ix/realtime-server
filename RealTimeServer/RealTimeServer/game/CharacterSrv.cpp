#include "RealTimeSrvPCH.h"


CharacterSrv::CharacterSrv() :
	mCatControlType( ESCT_Human ),
	mTimeOfNextShot( 0.f ),
	mTimeBetweenShots( 0.2f )
{}

void CharacterSrv::HandleDying()
{
	NetworkMgrSrv::sInst->UnregisterGameObject( this );
}

void CharacterSrv::Update()
{
	Character::Update();

	Vector3 oldLocation = GetLocation();
	Vector3 oldVelocity = GetVelocity();
	Vector3 oldRotation = GetRotation();

	ClientProxyPtr client = NetworkMgrSrv::sInst->GetClientProxy( GetPlayerId() );
	if ( client )
	{
		ActionList& moveList = client->GetUnprocessedMoveList();
		for ( const Action& unprocessedMove : moveList )
		{
			const InputState& currentState = unprocessedMove.GetInputState();
			float deltaTime = unprocessedMove.GetDeltaTime();
			ProcessInput( deltaTime, currentState );
			SimulateMovement( deltaTime );

			//LOG( " CharacterServer::SimulateMovement() GetPlayerId = %d", GetPlayerId() );
		}

		moveList.Clear();
	}

	//HandleShooting();


	//LOG( "GetLocation = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );
	//LOG( "GetRotation = %f, %f, %f", GetRotation().X, GetRotation().Y, GetRotation().Z );
	//LOG( "GetVelocity = %f, %f, %f", GetVelocity().X, GetVelocity().Y, GetVelocity().Z );

	if ( !RealTimeSrvMath::Is3DVectorEqual( oldLocation, GetLocation() ) ||
		!RealTimeSrvMath::Is3DVectorEqual( oldVelocity, GetVelocity() ) ||
		!RealTimeSrvMath::Is3DVectorEqual( oldRotation, GetRotation() )
		)
	{
		//LOG( " NetworkManagerServer::sInstance->SetStateDirty( GetNetworkId(), ECRS_Pose );  GetPlayerId = %d", GetPlayerId() );
		NetworkMgrSrv::sInst->SetStateDirty( GetNetworkId(), ECRS_Pose );
	}
}

void CharacterSrv::HandleShooting()
{
}

void CharacterSrv::TakeDamage( int inDamagingPlayerId )
{
}


