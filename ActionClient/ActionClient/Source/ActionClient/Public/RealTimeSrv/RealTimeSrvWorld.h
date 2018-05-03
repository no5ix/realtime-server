// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <memory>
#include <vector>
#include "RealTimeSrvEntity.h"

/**
 * 
 */
class RealTimeSrvWorld
{
public:

	static void StaticInit();

	static std::unique_ptr< RealTimeSrvWorld >		sInstance;

	void AddGameObject( GameObjectPtr inGameObject );
	void RemoveGameObject( GameObjectPtr inGameObject );

	void Update();

	const std::vector< GameObjectPtr >&	GetGameObjects()	const { return mGameObjects; }

private:


	RealTimeSrvWorld();

	int	GetIndexOfGameObject( GameObjectPtr inGameObject );

	std::vector< GameObjectPtr >	mGameObjects;


};
