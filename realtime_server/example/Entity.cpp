#include "RealTimeSrvPCH.h"




Entity::Entity() :
	mDoesWantToDie( false ),
	mIndexInWorld( -1 ),
	mNetworkId( 0 ),
	mCollisionRadius( 0.5f ),
	mScale( 1.0f ),
	mLocation( Vector3::Zero() ),
	mRotation( Vector3::Zero() )
	//mColor( Colors::White ),
{
}

void Entity::Update()
{
	//object don't do anything by default...	
}


//Vector3 GameObject::GetForwardVector()	const
//{
//	//should we cache this when you turn?
//	return Vector3( sinf( mRotation ), -cosf( mRotation ), 0.f );
//}

void Entity::SetNetworkId( int inNetworkId )
{
	//this doesn't put you in the map or remove you from it
	mNetworkId = inNetworkId;

}

//void GameObject::SetRotation( float inRotation )
//{
//	//should we normalize using fmodf?
//	mRotation = inRotation;
//}
