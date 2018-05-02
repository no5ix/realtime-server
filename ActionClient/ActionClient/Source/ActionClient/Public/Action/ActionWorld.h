// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <memory>
#include <vector>
#include "ActionEntity.h"

/**
 * 
 */
class ActionWorld
{
public:

	static void StaticInit();

	static std::unique_ptr< ActionWorld >		sInstance;

	void AddGameObject( GameObjectPtr inGameObject );
	void RemoveGameObject( GameObjectPtr inGameObject );

	void Update();

	const std::vector< GameObjectPtr >&	GetGameObjects()	const { return mGameObjects; }

private:


	ActionWorld();

	int	GetIndexOfGameObject( GameObjectPtr inGameObject );

	std::vector< GameObjectPtr >	mGameObjects;

	float mTimeOfLastUpdateTargetState;

	bool mIsTimeToUpdateTargetState;

};
