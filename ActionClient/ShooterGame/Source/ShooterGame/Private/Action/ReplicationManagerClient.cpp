// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "ReplicationManagerClient.h"



void ReplicationManagerClient::Read( InputMemoryBitStream& inInputStream )
{
	//while (inInputStream.GetRemainingBitCount() >= 34)
	//{
	//	//read the network id...
	//	int networkId; 
	//	inInputStream.Read( networkId );

	//	//only need 2 bits for action...
	//	uint8_t action; 
	//	inInputStream.Read( action, 2 );

	//	switch (action)
	//	{
	//	case RA_Create:
	//		ReadAndDoCreateAction( inInputStream, networkId );
	//		break;
	//	case RA_Update:
	//		ReadAndDoUpdateAction( inInputStream, networkId );
	//		break;
	//	case RA_Destroy:
	//		ReadAndDoDestroyAction( inInputStream, networkId );
	//		break;
	//	}

	//}


	//////////////


	//AActionPlayerController* const FirstPC = Cast<AActionPlayerController>( UGameplayStatics::GetPlayerController( GetWorld(), 0 ) );
	//if (FirstPC != nullptr)
	//{
	//	FRotator Rot( 0.f, 0.f, 0.f );
	//	FTransform SpawnTransform( Rot, FVector::ZeroVector );

	//	TSubclassOf<class AActionCharacter> CharacterClass;
	//	AActionCharacter* DeferredActor = Cast<AActionCharacter>( UGameplayStatics::BeginDeferredActorSpawnFromClass( this, CharacterClass, SpawnTransform ) );
	//	if (DeferredActor)
	//	{
	//		UGameplayStatics::FinishSpawningActor( DeferredActor, SpawnTransform );
	//	}
	//	FirstPC->Possess( DeferredActor );

	//}

}

void ReplicationManagerClient::ReadAndDoCreateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{
	//need 4 cc
	uint32_t fourCCName;
	inInputStream.Read( fourCCName );

	//we might already have this object- could happen if our ack of the create got dropped so server resends create request 
	//( even though we might have created )


	//GameObjectPtr gameObject = NetworkManager::sInstance->GetGameObject( inNetworkId );
	//if (!gameObject)
	//{
	//	//create the object and map it...
	//	gameObject = GameObjectRegistry::sInstance->CreateGameObject( fourCCName );
	//	gameObject->SetNetworkId( inNetworkId );
	//	NetworkManagerClient::sInstance->AddToNetworkIdToGameObjectMap( gameObject );


		//FRotator Rot( 0.f, 0.f, 0.f );
		//FTransform SpawnTransform( Rot, FVector::ZeroVector );

		//TSubclassOf<class AActionCharacter> CharacterClass;
		//AActionCharacter* DeferredActor = Cast<AActionCharacter>( UGameplayStatics::BeginDeferredActorSpawnFromClass( this, CharacterClass, SpawnTransform ) );
		//if (DeferredActor)
		//{

		//	UGameplayStatics::FinishSpawningActor( DeferredActor, SpawnTransform );
		//}

	//	//it had really be the rigth type...
	//	assert( gameObject->GetClassId() == fourCCName );
	//}

	////and read state
	//gameObject->Read( inInputStream );

	//DeferredActor->Read( inInputStream );

	////////////////


}

void ReplicationManagerClient::ReadAndDoUpdateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{
//	//need object
//	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject( inNetworkId );
//
//	//gameObject MUST be found, because create was ack'd if we're getting an update...
//	//and read state
//	gameObject->Read( inInputStream );
}

void ReplicationManagerClient::ReadAndDoDestroyAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{
//	//if something was destroyed before the create went through, we'll never get it
//	//but we might get the destroy request, so be tolerant of being asked to destroy something that wasn't created
//	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject( inNetworkId );
//	if (gameObject)
//	{
//		gameObject->SetDoesWantToDie( true );
//		NetworkManagerClient::sInstance->RemoveFromNetworkIdToGameObjectMap( gameObject );
//	}
}