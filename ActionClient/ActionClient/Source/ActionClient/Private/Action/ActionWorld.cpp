// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionWorld.h"
#include "NetworkManager.h"
#include "ActionHelper.h"




std::unique_ptr< ActionWorld >	ActionWorld::sInstance;

void ActionWorld::StaticInit()
{
	sInstance.reset( new ActionWorld() );
}

ActionWorld::ActionWorld()
{
}


void ActionWorld::AddGameObject( GameObjectPtr inGameObject )
{
	mGameObjects.push_back( inGameObject );
	inGameObject->SetIndexInWorld( mGameObjects.size() - 1 );
}


void ActionWorld::RemoveGameObject( GameObjectPtr inGameObject )
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


void ActionWorld::Update()
{
	//update all game objects- sometimes they want to die, so we need to tread carefully...

	for ( int i = 0, c = mGameObjects.size(); i < c; ++i )
	{
		GameObjectPtr go = mGameObjects[i];

		if ( !go->DoesWantToDie() )
		{
			go->Update();
		}
		//you might suddenly want to die after your update, so check again
		if ( go->DoesWantToDie() )
		{
			RemoveGameObject( go );
			go->HandleDying();
			--i;
			--c;
		}
	}
}