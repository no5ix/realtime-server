#include "ActionServerPCH.h"


CharacterServer::CharacterServer() :
	mCatControlType( ESCT_Human ),
	mTimeOfNextShot( 0.f ),
	mTimeBetweenShots( 0.2f )
{}

void CharacterServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject( this );
}

void CharacterServer::Update()
{
	Character::Update();

	Vector3 oldLocation = GetLocation();
	Vector3 oldVelocity = GetVelocity();
	Vector3 oldRotation = GetRotation();

	ClientProxyPtr client = NetworkManagerServer::sInstance->GetClientProxy( GetPlayerId() );
	if (client)
	{
		MoveList& moveList = client->GetUnprocessedMoveList();
		for (const Move& unprocessedMove : moveList)
		{
			const InputState& currentState = unprocessedMove.GetInputState();
			float deltaTime = unprocessedMove.GetDeltaTime();
			ProcessInput( deltaTime, currentState );
			SimulateMovement( deltaTime );
		}

		moveList.Clear();
	}

	//HandleShooting();

	if ( !ActionServerMath::Is3DVectorEqual( oldLocation, GetLocation() ) ||
		 !ActionServerMath::Is3DVectorEqual( oldVelocity, GetVelocity() ) ||
		 !ActionServerMath::Is3DVectorEqual( oldRotation, GetRotation() ) 
		)
	{
		NetworkManagerServer::sInstance->SetStateDirty( GetNetworkId(), ECRS_Pose );
	}
}

void CharacterServer::HandleShooting()
{
}

void CharacterServer::TakeDamage( int inDamagingPlayerId )
{
}


