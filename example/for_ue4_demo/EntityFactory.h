#pragma once


typedef GameObjPtr( *GameObjectCreationFunc )( );

class EntityFactory
{
public:

	static void StaticInit();

	static std::unique_ptr< EntityFactory >		sInst;

	void RegisterCreationFunction( uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction );

	GameObjPtr CreateGameObject( uint32_t inFourCCName );

private:

	EntityFactory();

	unordered_map< uint32_t, GameObjectCreationFunc >	mNameToGameObjectCreationFunctionMap;

};
