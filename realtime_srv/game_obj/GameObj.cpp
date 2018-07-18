#include "realtime_srv/common/RealtimeSrvShared.h"
//;;;
#include <muduo/base/Logging.h>

using namespace realtime_srv;

GameObj::GameObj() :
	isPendingToDie_( false ),
	hasOwner_( false ),
	objId_( 0 )
{}

void GameObj::SetStateDirty( uint32_t repState )
{
	assert( networkMgr_ );
	LOG_INFO << "SetStateDirtySetStateDirtySetStateDirty";
	networkMgr_->SetRepStateDirty( objId_, repState );
}

void GameObj::Update()
{

	BeforeProcessInput();

	if ( hasOwner_ )
	{
		ActionList& actionList = GetOwner()->GetUnprocessedActionList();
		for ( const Action& unprocessedAction : actionList )
		{
			const InputStatePtr& currentState = unprocessedAction.GetInputState();
			float deltaTime = unprocessedAction.GetDeltaTime();
			ProcessInput( deltaTime, currentState );
		}
		actionList.Clear();
	}

	AfterProcessInput();
}