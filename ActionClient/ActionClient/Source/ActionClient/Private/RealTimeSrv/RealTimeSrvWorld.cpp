// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "RealTimeSrvWorld.h"
#include "NetworkMgr.h"
#include "RealTimeSrvHelper.h"




std::unique_ptr< RealTimeSrvWorld >	RealTimeSrvWorld::sInstance;

void RealTimeSrvWorld::StaticInit()
{
	sInstance.reset( new RealTimeSrvWorld() );
}

RealTimeSrvWorld::RealTimeSrvWorld()
{
}


void RealTimeSrvWorld::AddGameObject( GameObjectPtr inGameObject )
{
	mGameObjects.push_back( inGameObject );
	inGameObject->SetIndexInWorld( mGameObjects.size() - 1 );
}


void RealTimeSrvWorld::RemoveGameObject( GameObjectPtr inGameObject )
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


void RealTimeSrvWorld::Update()
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