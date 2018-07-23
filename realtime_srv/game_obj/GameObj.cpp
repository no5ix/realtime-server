#include "realtime_srv/net/ClientProxy.h"
#include "realtime_srv/net/NetworkMgr.h"

#include "realtime_srv/game_obj/GameObj.h"

using namespace realtime_srv;

GameObj::GameObj() :
	isPendingToDie_( false ),
	hasMaster_( false ),
	objId_( 0 )
{}

void GameObj::SetStateDirty( uint32_t repState )
{
	assert( networkMgr_ );
	networkMgr_->SetRepStateDirty( objId_, repState );
}

void GameObj::Update()
{
	BeforeProcessInput();

	if ( hasMaster_ )
	{
		ActionList& actionList = GetMaster()->GetUnprocessedActionList();
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

void GameObj::LoseMaster()
{
	if ( ClientProxyPtr m = master_.lock() )
		m->RealeaseSpecificOwnedGameObj( objId_ );
	hasMaster_ = false;
	master_.reset();
}


void GameObj::SetMaster( std::shared_ptr<ClientProxy> cp )
{ master_ = cp; hasMaster_ = true; cp->AddGameObj( shared_from_this() ); }


void GameObj::SetPendingToDie()
{ isPendingToDie_ = true; LoseMaster(); }
