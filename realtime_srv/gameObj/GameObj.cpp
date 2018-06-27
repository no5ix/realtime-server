#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;

GameObj::GameObj() :
	mDoesWantToDie( false ),
	ObjId_( 0 ),
	PlayerId_( 0 ),
	currentLocation_( Vector3::Zero() ),
	oldLocation_( Vector3::Zero() ),
	currentRotation_( Vector3::Zero() ),
	oldRotation_( Vector3::Zero() ) {}

void GameObj::SetOldState() {
	oldLocation_ = GetLocation();
	oldRotation_ = GetRotation();
}

bool GameObj::IsStateDirty() {
	return !RealtimeSrvMath::Is3DVectorEqual( oldLocation_, GetLocation() ) ||
		!RealtimeSrvMath::Is3DVectorEqual( oldRotation_, GetRotation() );
}

void GameObj::SetStateDirty( uint32_t repState ) {
	if ( ClientProxyPtr cp = GetClientProxy() ) {
		cp->SetGameObjStateDirty( GetObjId(), repState );
	}
}

void GameObj::Update() {
	if ( ClientProxyPtr cp = GetClientProxy() ) {
		ActionList& actionList = cp->GetUnprocessedActionList();
		for ( const Action& unprocessedAction : actionList ) {
			const InputState& currentState = unprocessedAction.GetInputState();
			float deltaTime = unprocessedAction.GetDeltaTime();
			ProcessInput( deltaTime, currentState );
		}
		actionList.Clear();
	}
}