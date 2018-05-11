// Fill out your copyright notice in the Description page of Project Settings.


#include "RealTimeSrvEntityFactory.h"
#include "RealTimeSrvWorld.h"

std::unique_ptr<RealTimeSrvEntityFactory> RealTimeSrvEntityFactory::sInstance;

RealTimeSrvEntityFactory::RealTimeSrvEntityFactory()
{
}


void RealTimeSrvEntityFactory::StaticInit(UWorld * inWorld)
{
	sInstance.reset( new RealTimeSrvEntityFactory() );
	check( sInstance );
	if (sInstance)
	{
		sInstance->mWorld = inWorld;
	}
}

UWorld* RealTimeSrvEntityFactory::GetWorld() const
{
	return mWorld;
}



RealTimeSrvEntityPtr RealTimeSrvEntityFactory::CreateGameObject( uint32_t inFourCCName )
{

	switch ( inFourCCName )
	{
	case 'CHRT':
		return CreateRealTimeSrvPawn();
	default:
		break;
	}

	return RealTimeSrvEntityPtr();
}

RealTimeSrvEntityPtr RealTimeSrvEntityFactory::CreateRealTimeSrvPawn()
{
	check( GetWorld() );

	UWorld* const world = GetWorld();

	if ( world )
	{
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ARealTimeSrvPawn* const newPawn = world->SpawnActor<ARealTimeSrvPawn>( DefaultCharacterClasses, FTransform::Identity, SpawnParams );


		if ( newPawn )
		{
			RealTimeSrvWorld::sInstance->AddGameObject( newPawn );
		}
		return RealTimeSrvEntityPtr( newPawn );

	}
	return RealTimeSrvEntityPtr();
}