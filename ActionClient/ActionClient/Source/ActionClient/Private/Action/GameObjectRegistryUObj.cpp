// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "GameObjectRegistryUObj.h"
#include "ActionPlayerController.h"
#include "ActionPawn.h"
#include "NetworkManager.h"


UGameObjectRegistryUObj::UGameObjectRegistryUObj()
{
	//RegisterCreationFunction( 'CHRT', CreateActionPawn );
}


UWorld* UGameObjectRegistryUObj::GetWorld() const
{
	return GWorld;
}

void UGameObjectRegistryUObj::RegisterCreationFunction( uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction )
{
	mNameToGameObjectCreationFunctionMap[inFourCCName] = inCreationFunction;
}

GameObjectPtr UGameObjectRegistryUObj::CreateGameObject( uint32_t inFourCCName )
{

	//GameObjectCreationFunc creationFunc = mNameToGameObjectCreationFunctionMap[inFourCCName];
	switch (inFourCCName)
	{
	case 'CHRT':
		return CreateActionPawn();
	default:
		break;
	}

	//GameObjectPtr gameObject = creationFunc();

	//World::sInstance->AddGameObject( gameObject );

	return GameObjectPtr();
}

GameObjectPtr UGameObjectRegistryUObj::CreateActionPawn()
{

	UWorld* const World = GetWorld();

	if (World)
	{
		//AActionPlayerController* const FirstPC = Cast<AActionPlayerController>( UGameplayStatics::GetPlayerController( GetWorld(), 0 ) );
		//if (FirstPC != nullptr)
		//{

			//FRotator Rot( 0.f, 0.f, 0.f );
			//FTransform SpawnTransform( Rot, FVector::ZeroVector );

			//TSubclassOf<class AActionCharacter> CharacterClass;
			//AActionCharacter* DeferredActor = Cast<AActionCharacter>( UGameplayStatics::BeginDeferredActorSpawnFromClass( this, CharacterClass, SpawnTransform ) );
			//if (DeferredActor)
			//{
			//	UGameplayStatics::FinishSpawningActor( DeferredActor, SpawnTransform );
			//}

			//TSubclassOf<AActor> ActorClass

			//TSubclassOf<class AActionCharacter> DefaultCharacterClasses;


			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AActionPawn* const newActionPawn = World->SpawnActor<AActionPawn>( DefaultCharacterClasses, FTransform::Identity, SpawnParams );

		//	if (inPlayerID == NetworkManager::sInstance->GetPlayerId())
		//	{
		//		FirstPC->Possess( newActionPawn );
		//	}

			return GameObjectPtr( newActionPawn );
		//}
	}
	return GameObjectPtr();
}