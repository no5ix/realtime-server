#pragma once

enum ECatControlType
{
	ESCT_Human,
	ESCT_AI
};

// CharacterServer
class CharacterSrv : public Character
{
public:
	static EntityPtr	StaticCreate() { return NetworkMgrSrv::sInst->RegisterAndReturn( new CharacterSrv() ); }
	virtual void HandleDying() override;

	virtual void Update() override;

	void SetCatControlType( ECatControlType inCatControlType ) { mCatControlType = inCatControlType; }

	void TakeDamage( int inDamagingPlayerId );

protected:
	CharacterSrv();

private:

	void HandleShooting();

	ECatControlType	mCatControlType;

	
	float		mTimeOfNextShot;
	float		mTimeBetweenShots;

};