#include "realtime_srv/common/RealtimeSrvShared.h"




GameObj::GameObj() :
	mDoesWantToDie( false ),
	mIndexInWorld( -1 ),
	mNetworkId( 0 ),
	mLocation( Vector3::Zero() ),
	mRotation( Vector3::Zero() )
{
}

void GameObj::SetNetworkId( int inNetworkId )
{
	//this doesn't put you in the map or remove you from it
	mNetworkId = inNetworkId;

}
