// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionEntityFactory.h"

std::unique_ptr<ActionEntityFactory> ActionEntityFactory::sInstance;

ActionEntityFactory::ActionEntityFactory()
{
}


void ActionEntityFactory::StaticInit(UWorld * inWorld)
{
	sInstance.reset( new ActionEntityFactory() );
	check( sInstance );
	if (sInstance)
	{
		sInstance->mWorld = inWorld;
	}
}

UWorld* ActionEntityFactory::GetWorld() const
{
	return mWorld;
}



GameObjectPtr ActionEntityFactory::CreateGameObject( uint32_t inFourCCName )
{

	switch ( inFourCCName )
	{
	case 'CHRT':
		return CreateActionPawn();
	default:
		break;
	}

	return GameObjectPtr();
}

GameObjectPtr ActionEntityFactory::CreateActionPawn()
{
	check( GetWorld() );

	UWorld* const world = GetWorld();

	if ( world )
	{
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActionPawn* const newActionPawn = world->SpawnActor<AActionPawn>( DefaultCharacterClasses, FTransform::Identity, SpawnParams );


		if ( newActionPawn )
		{
			ActionWorld::sInstance->AddGameObject( newActionPawn );
		}
		return GameObjectPtr( newActionPawn );

	}
	return GameObjectPtr();
}