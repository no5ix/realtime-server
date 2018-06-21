#include "realtime_srv/common/RealTimeSrvShared.h"




Entity::Entity() :
	mDoesWantToDie( false ),
	mIndexInWorld( -1 ),
	mNetworkId( 0 ),
	mCollisionRadius( 0.5f ),
	mScale( 1.0f ),
	mLocation( Vector3::Zero() ),
	mRotation( Vector3::Zero() )
{
}

void Entity::SetNetworkId( int inNetworkId )
{
	//this doesn't put you in the map or remove you from it
	mNetworkId = inNetworkId;

}
