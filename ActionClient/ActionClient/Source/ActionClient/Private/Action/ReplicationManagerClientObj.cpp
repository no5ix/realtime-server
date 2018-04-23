// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ReplicationManagerClientObj.h"
#include "ActionPlayerController.h"




void UReplicationManagerClientObj::Read( InputMemoryBitStream& inInputStream )
{
	while (inInputStream.GetRemainingBitCount() >= 34)
	{
		//read the network id...
		int networkId; 
		inInputStream.Read( networkId );

		//only need 2 bits for action...
		uint8_t action; 
		inInputStream.Read( action, 2 );

		switch (action)
		{
		case RA_Create:
			ReadAndDoCreateAction( inInputStream, networkId );
			break;
		case RA_Update:
			ReadAndDoUpdateAction( inInputStream, networkId );
			break;
		case RA_Destroy:
			ReadAndDoDestroyAction( inInputStream, networkId );
			break;
		}

	}

	/////
}

UWorld* UReplicationManagerClientObj::GetWorld() const
{
	return GWorld;
}

void UReplicationManagerClientObj::ReadAndDoCreateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

	////need 4 cc
	//uint32_t fourCCName;
	//inInputStream.Read( fourCCName );

	////we might already have this object- could happen if our ack of the create got dropped so server resends create request 
	////( even though we might have created )
	//GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject( inNetworkId );
	//if (!gameObject)
	//{
	//	//create the object and map it...
	//	gameObject = GameObjectRegistry::sInstance->CreateGameObject( fourCCName );
	//	gameObject->SetNetworkId( inNetworkId );
	//	NetworkManagerClient::sInstance->AddToNetworkIdToGameObjectMap( gameObject );

	//	//it had really be the rigth type...
	//	assert( gameObject->GetClassId() == fourCCName );
	//}

	////and read state
	//gameObject->Read( inInputStream );

	/////////////////////////////////////////////////////////////////

	//UWorld* const World = GetWorld();

	//if (World)
	//{
	//	AActionPlayerController* const FirstPC = Cast<AActionPlayerController>( UGameplayStatics::GetPlayerController( GetWorld(), 0 ) );
	//	if (FirstPC != nullptr)
	//	{

	//		//FRotator Rot( 0.f, 0.f, 0.f );
	//		//FTransform SpawnTransform( Rot, FVector::ZeroVector );

	//		//TSubclassOf<class AActionCharacter> CharacterClass;
	//		//AActionCharacter* DeferredActor = Cast<AActionCharacter>( UGameplayStatics::BeginDeferredActorSpawnFromClass( this, CharacterClass, SpawnTransform ) );
	//		//if (DeferredActor)
	//		//{
	//		//	UGameplayStatics::FinishSpawningActor( DeferredActor, SpawnTransform );
	//		//}

	//		//TSubclassOf<AActor> ActorClass
	//		FActorSpawnParameters SpawnParams;
	//		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//		AActionCharacter* const NewActionCharacter = World->SpawnActor<AActionCharacter>( DefaultCharacterClasses, FTransform::Identity, SpawnParams );


	//		FirstPC->Possess( NewActionCharacter );
	//	}
	//}

}

void UReplicationManagerClientObj::ReadAndDoUpdateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

}

void UReplicationManagerClientObj::ReadAndDoDestroyAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

}
