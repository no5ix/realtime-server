#pragma once

#define CLASS_IDENTIFICATION( inCode, inClass ) \
enum { kClassId = inCode }; \
virtual uint32_t GetClassId() const { return kClassId; } \
static inClass* CreateInstance() { return static_cast< inClass* >( new inClass() ); } \

class GameObj
{
public:
	// 'GOBJ' = 1196376650;
	CLASS_IDENTIFICATION( 'GOBJ', GameObj )

	GameObj();
	virtual ~GameObj() {}
	virtual uint32_t GetAllStateMask()	const { return 0; }

	virtual uint32_t GetMaxSerializeSize() const{ return 0; }

	virtual void	Update() {}

	virtual void	HandleDying() {}

	void	SetIndexInWorld( int inIndex ) { mIndexInWorld = inIndex; }
	int		GetIndexInWorld()				const { return mIndexInWorld; }

	void	SetRotation( Vector3 inRotation ) { mRotation = inRotation; }
	const Vector3&	GetRotation()					const { return mRotation; }


	const Vector3&		GetLocation()				const { return mLocation; }
	void		SetLocation( const Vector3& inLocation ) { mLocation = inLocation; }

	bool		DoesWantToDie()				const { return mDoesWantToDie; }
	void		SetDoesWantToDie( bool inWants ) { mDoesWantToDie = inWants; }

	int			GetNetworkId()				const { return mNetworkId; }
	void		SetNetworkId( int inNetworkId );

	virtual uint32_t	Write( OutputBitStream& inOutputStream, uint32_t inDirtyState ) const { ( void )inOutputStream; ( void )inDirtyState; return 0; }
	virtual void		Read( InputBitStream& inInputStream ) { ( void )inInputStream; }

protected:

	bool											mDoesWantToDie;

	int												mIndexInWorld;
	int												mNetworkId;


	Vector3											mLocation;
	Vector3											mRotation;
};

typedef shared_ptr< GameObj >	GameObjPtr;