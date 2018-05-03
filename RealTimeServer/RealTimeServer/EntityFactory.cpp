#include "RealTimeServerPCH.h"



std::unique_ptr< EntityFactory >	EntityFactory::sInstance;

void EntityFactory::StaticInit()
{
	sInstance.reset( new EntityFactory() );
}

EntityFactory::EntityFactory()
{
}

void EntityFactory::RegisterCreationFunction( uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction )
{
	mNameToGameObjectCreationFunctionMap[inFourCCName] = inCreationFunction;
}

GameObjectPtr EntityFactory::CreateGameObject( uint32_t inFourCCName )
{
	
	GameObjectCreationFunc creationFunc = mNameToGameObjectCreationFunctionMap[inFourCCName];

	GameObjectPtr gameObject = creationFunc();

	
	
	World::sInstance->AddGameObject( gameObject );

	return gameObject;
}