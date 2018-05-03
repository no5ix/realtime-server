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

	void AddGameObject( RealTimeSrvEntityPtr inGameObject );
	void RemoveGameObject( RealTimeSrvEntityPtr inGameObject );

	void Update();

	const std::vector< RealTimeSrvEntityPtr >&	GetGameObjects()	const { return mGameObjects; }

private:


	RealTimeSrvWorld();

	int	GetIndexOfGameObject( RealTimeSrvEntityPtr inGameObject );

	std::vector< RealTimeSrvEntityPtr >	mGameObjects;


};
