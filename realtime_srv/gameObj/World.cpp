#include "realtime_srv/common/RealtimeSrvShared.h"





std::unique_ptr< World >	World::sInstance;

void World::StaticInit()
{
	sInstance.reset( new World() );
}


#ifdef IS_LINUX

World::World()
	: GameObjects_( new GameObjs )
{}

//GameObjectsPtr World::GetGameObjects()
//{
//	MutexLockGuard lock( mutex_ );
//	return GameObjects_;
//}
//
//void World::GameObjectsCOW()
//{
//	if ( !GameObjects_.unique() )
//	{
//		GameObjects_.reset( new GameObjs( *GameObjects_ ) );
//	}
//	assert( GameObjects_.unique() );
//}

void World::AddGameObject( GameObjPtr inGameObject )
{
	//MutexLockGuard lock( mutex_ );
	//GameObjectsCOW();
	//GameObjects_->insert( inGameObject );

	auto tempFunc = [&]() { GameObjects_->insert( inGameObject ); };
	SET_THREAD_SHARED_VAR( GameObjects_, mutex_, tempFunc );
}

void World::RemoveGameObject( GameObjPtr inGameObject )
{
	//MutexLockGuard lock( mutex_ );
	//GameObjectsCOW();
	//GameObjects_->erase( inGameObject );

	auto tempFunc = [&]() { GameObjects_->erase( inGameObject ); };
	SET_THREAD_SHARED_VAR( GameObjects_, mutex_, tempFunc );
}

void World::Update()
{
	vector< GameObjPtr > GameObjsToRem;
	GameObjectsPtr  tempGameObjects = GET_THREAD_SHARED_VAR( GameObjects_ );

	//for ( GameObjs::iterator go = tempGameObjects->begin();
	//	go != tempGameObjects->end(); ++go )
	for ( auto& go : *tempGameObjects )
	{
		if ( !( go )->DoesWantToDie() )
		{
			( go )->Update();
		}
		else
		{
			GameObjsToRem.push_back( go );
			( go )->HandleDying();
		}
	}

	if ( GameObjsToRem.size() > 0 )
	{
		//MutexLockGuard lock( mutex_ );
		//GameObjectsCOW();
		auto tempFunc = [&]()
		{
			for ( auto g : GameObjsToRem )
				GameObjects_->erase( g );
		};
		SET_THREAD_SHARED_VAR( GameObjects_, mutex_, tempFunc );
	}
}

#else //IS_LINUX

World::World()
{}

void World::AddGameObject( GameObjPtr inGameObject )
{
	GameObjects_.push_back( inGameObject );
	inGameObject->SetIndexInWorld( GameObjects_.size() - 1 );
}


void World::RemoveGameObject( GameObjPtr inGameObject )
{
	int index = inGameObject->GetIndexInWorld();

	int lastIndex = GameObjects_.size() - 1;
	if ( index != lastIndex )
	{
		GameObjects_[index] = GameObjects_[lastIndex];
		GameObjects_[index]->SetIndexInWorld( index );
	}

	inGameObject->SetIndexInWorld( -1 );

	GameObjects_.pop_back();
}


void World::Update()
{
	for ( int i = 0, c = GameObjects_.size(); i < c; ++i )
	{
		GameObjPtr go = GameObjects_[i];
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
#endif //IS_LINUX
