#include "RealTimeSrvPCH.h"



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

EntityPtr EntityFactory::CreateGameObject( uint32_t inFourCCName )
{
	GameObjectCreationFunc creationFunc = mNameToGameObjectCreationFunctionMap[inFourCCName];

	EntityPtr gameObject = creationFunc();
	
	World::sInst->AddGameObject( gameObject );

	return gameObject;
}