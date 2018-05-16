#pragma once


/*
* the world tracks all the live game objects. Fairly inefficient for now, but not that much of a problem
*/
class World
{

public:

	static void StaticInit();

	static std::unique_ptr< World >		sInst;

	void AddGameObject( EntityPtr inGameObject );
	void RemoveGameObject( EntityPtr inGameObject );

	void Update();

	const std::vector< EntityPtr >&	GetGameObjects()	const { return mGameObjects; }

private:


	World();

	//int	GetIndexOfGameObject( GameObjectPtr inGameObject );

	std::vector< EntityPtr >	mGameObjects;


};