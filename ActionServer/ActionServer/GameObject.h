#pragma once

#define CLASS_IDENTIFICATION( inCode, inClass ) \
enum { kClassId = inCode }; \
virtual uint32_t GetClassId() const { return kClassId; } \
static GameObject* CreateInstance() { return static_cast< GameObject* >( new inClass() ); } \

class GameObject
{
public:
	// 'GOBJ' = 1196376650;
	CLASS_IDENTIFICATION( 'GOBJ', GameObject )

	GameObject();
//
	virtual ~GameObject() {}
//
//	virtual	RoboCat*	GetAsCat() { return nullptr; }
//
	virtual uint32_t GetAllStateMask()	const { return 0; }
//
//	//return whether to keep processing collision
//	virtual bool	HandleCollisionWithCat( RoboCat* inCat ) { ( void )inCat; return true; }

	virtual void	Update();

	virtual void	HandleDying() {}

	void	SetIndexInWorld( int inIndex ) { mIndexInWorld = inIndex; }
	int		GetIndexInWorld()				const { return mIndexInWorld; }

	void	SetRotation( Vector3 inRotation ) { mRotation = inRotation; }
	Vector3	GetRotation()					const { return mRotation; }

	void	SetScale( float inScale ) { mScale = inScale; }
	float	GetScale()						const { return mScale; }

	const Vector3&		GetLocation()				const { return mLocation; }
	void		SetLocation( const Vector3& inLocation ) { mLocation = inLocation; }


//	float		GetCollisionRadius()		const { return mCollisionRadius; }
//	void		SetCollisionRadius( float inRadius ) { mCollisionRadius = inRadius; }
//
//	Vector3		GetForwardVector()			const;
//
//

	bool		DoesWantToDie()				const { return mDoesWantToDie; }
	void		SetDoesWantToDie( bool inWants ) { mDoesWantToDie = inWants; }

	int			GetNetworkId()				const { return mNetworkId; }
	void		SetNetworkId( int inNetworkId );

	virtual uint32_t	Write( OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState ) const { ( void )inOutputStream; ( void )inDirtyState; return 0; }
	virtual void		Read( InputMemoryBitStream& inInputStream ) { ( void )inInputStream; }

protected:

	Vector3											mLocation;
	//Vector3											mColor;

	float											mCollisionRadius;


	Vector3											mRotation;
	float											mScale;
	int												mIndexInWorld;

	bool											mDoesWantToDie;

	int												mNetworkId;

};

typedef shared_ptr< GameObject >	GameObjectPtr;