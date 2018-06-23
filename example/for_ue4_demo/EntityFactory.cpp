#include "for_ue4_demo_shared.h"



std::unique_ptr< EntityFactory >	EntityFactory::sInst;

void EntityFactory::StaticInit()
{
	sInst.reset( new EntityFactory() );
}

EntityFactory::EntityFactory()
{
}

void EntityFactory::RegisterCreationFunction( uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction )
{
	mNameToGameObjectCreationFunctionMap[inFourCCName] = inCreationFunction;
}

GameObjPtr EntityFactory::CreateGameObject( uint32_t inFourCCName )
{
	GameObjectCreationFunc creationFunc = mNameToGameObjectCreationFunctionMap[inFourCCName];

	GameObjPtr gameObject = creationFunc();
	
	World::sInst->AddGameObject( gameObject );

	return gameObject;
}