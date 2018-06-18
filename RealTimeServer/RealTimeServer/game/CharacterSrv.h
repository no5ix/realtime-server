#pragma once

// CharacterServer
class CharacterSrv : public Character
{
public:
	static EntityPtr	StaticCreate() 
	{ return NetworkMgrSrv::sInst->RegisterAndReturn( new CharacterSrv() ); }
	virtual void HandleDying() override;

	virtual void Update() override;

	void TakeDamage( int inDamagingPlayerId );

protected:
	CharacterSrv();

private:

	void HandleShooting();

	float		mTimeOfNextShot;
	float		mTimeBetweenShots;

};