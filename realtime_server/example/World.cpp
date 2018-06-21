#include "RealTimeSrvPCH.h"





std::unique_ptr< World >	World::sInst;

void World::StaticInit()
{
	sInst.reset( new World() );
}


#ifdef NEW_EPOLL_INTERFACE

World::World()
	: mGameObjects( new GameObjs )
{}

GameObjectsPtr World::GetGameObjects()
{
	MutexLockGuard lock( mutex_ );
	return mGameObjects;
}

void World::GameObjectsCOW()
{
	if ( !mGameObjects.unique() )
	{
		mGameObjects.reset( new GameObjs( *mGameObjects ) );
	}
	assert( mGameObjects.unique() );
}

void World::AddGameObject( EntityPtr inGameObject )
{
	MutexLockGuard lock( mutex_ );
	GameObjectsCOW();
	mGameObjects->insert( inGameObject );
}

void World::RemoveGameObject( EntityPtr inGameObject )
{
	MutexLockGuard lock( mutex_ );
	GameObjectsCOW();
	mGameObjects->erase( inGameObject );
}

void World::Update()
{
	vector< EntityPtr > GameObjsToRem;
	GameObjectsPtr  tempGameObjects = GetGameObjects();

	for ( GameObjs::iterator go = tempGameObjects->begin();
		go != tempGameObjects->end(); ++go )
	//for (  auto& go : *tempGameObjects )
	{
		if ( !( *go )->DoesWantToDie() )
		{
			( *go )->Update();
		}
		else
		{
			GameObjsToRem.push_back( ( *go ) );
			( *go )->HandleDying();
		}
	}

	if ( GameObjsToRem.size() > 0 )
	{
		MutexLockGuard lock( mutex_ );
		GameObjectsCOW();
		for ( auto g : GameObjsToRem )
		{
			mGameObjects->erase( g );
		}
	}
}

#else //NEW_EPOLL_INTERFACE

World::World()
{}

void World::AddGameObject( EntityPtr inGameObject )
{
	mGameObjects.push_back( inGameObject );
	inGameObject->SetIndexInWorld( mGameObjects.size() - 1 );
}


void World::RemoveGameObject( EntityPtr inGameObject )
{
	int index = inGameObject->GetIndexInWorld();

	int lastIndex = mGameObjects.size() - 1;
	if ( index != lastIndex )
	{
		mGameObjects[index] = mGameObjects[lastIndex];
		mGameObjects[index]->SetIndexInWorld( index );
	}

	inGameObject->SetIndexInWorld( -1 );

	mGameObjects.pop_back();
}


void World::Update()
{
	for ( int i = 0, c = mGameObjects.size(); i < c; ++i )
	{
		EntityPtr go = mGameObjects[i];
		if ( !go->DoesWantToDie() )
		{
			go->Update();
		}
		if ( go->DoesWantToDie() )
		{
			RemoveGameObject( go );
			go->HandleDying();
			--i;
			--c;
		}
	}
}
#endif //NEW_EPOLL_INTERFACE
