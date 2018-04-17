#pragma once

enum ECatControlType
{
	ESCT_Human,
	ESCT_AI
};

// CharacterServer
class CharacterServer : public Character
{
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn( new CharacterServer() ); }
	virtual void HandleDying() override;

	virtual void Update() override;

	void SetCatControlType( ECatControlType inCatControlType ) { mCatControlType = inCatControlType; }

	void TakeDamage( int inDamagingPlayerId );

protected:
	CharacterServer();

private:

	void HandleShooting();

	ECatControlType	mCatControlType;

	
	float		mTimeOfNextShot;
	float		mTimeBetweenShots;

};