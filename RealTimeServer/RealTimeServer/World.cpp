#include "RealTimeSrvPCH.h"





std::unique_ptr< World >	World::sInst;

void World::StaticInit()
{
	sInst.reset( new World() );
}

World::World()
{
}


void World::AddGameObject( EntityPtr inGameObject )
{
	mGameObjects.push_back( inGameObject );
	inGameObject->SetIndexInWorld( mGameObjects.size() - 1 );
}


void World::RemoveGameObject( EntityPtr inGameObject )
{
	int index = inGameObject->GetIndexInWorld();

	int lastIndex = mGameObjects.size() - 1;
	if (index != lastIndex)
	{
		mGameObjects[index] = mGameObjects[lastIndex];
		mGameObjects[index]->SetIndexInWorld( index );
	}

	inGameObject->SetIndexInWorld( -1 );

	mGameObjects.pop_back();
}


void World::Update()
{

	for (int i = 0, c = mGameObjects.size(); i < c; ++i)
	{
		EntityPtr go = mGameObjects[i];


		if (!go->DoesWantToDie())
		{
			go->Update();
		}
		if (go->DoesWantToDie())
		{
			RemoveGameObject( go );
			go->HandleDying();
			--i;
			--c;
		}
	}
}