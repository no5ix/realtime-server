#pragma once


typedef EntityPtr( *GameObjectCreationFunc )( );

class EntityFactory
{
public:

	static void StaticInit();

	static std::unique_ptr< EntityFactory >		sInstance;

	void RegisterCreationFunction( uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction );

	EntityPtr CreateGameObject( uint32_t inFourCCName );

private:

	EntityFactory();

	unordered_map< uint32_t, GameObjectCreationFunc >	mNameToGameObjectCreationFunctionMap;

};
