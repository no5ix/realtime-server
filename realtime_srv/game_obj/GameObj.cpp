#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;

GameObj::GameObj() :
	isPendingToDie_( false ),
	ObjId_( 0 )
{}

void GameObj::SetStateDirty( uint32_t repState )
{
	if ( ClientProxyPtr cp = GetOwner() )
	{
		cp->SetGameObjStateDirty( GetObjId(), repState );
	}
}

void GameObj::Update()
{

	BeforeUpdate();

	if ( ClientProxyPtr cp = GetOwner() )
	{
		ActionList& actionList = cp->GetUnprocessedActionList();
		for ( const Action& unprocessedAction : actionList )
		{
			const InputStatePtr& currentState = unprocessedAction.GetInputState();
			float deltaTime = unprocessedAction.GetDeltaTime();
			ProcessInput( deltaTime, currentState );
		}
		actionList.Clear();
	}

	AfterUpdate();
}