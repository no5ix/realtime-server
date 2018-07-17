#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;

GameObj::GameObj() :
	isPendingToDestroy_( false ),
	ObjId_( 0 )
{}

void GameObj::SetStateDirty( uint32_t repState )
{
	if ( ClientProxyPtr cp = GetClientProxy() )
	{
		cp->SetGameObjStateDirty( GetObjId(), repState );
	}
}

void GameObj::Update()
{

	SetOldState();

	if ( ClientProxyPtr cp = GetClientProxy() )
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

	CheckAndSetDirtyState();
}