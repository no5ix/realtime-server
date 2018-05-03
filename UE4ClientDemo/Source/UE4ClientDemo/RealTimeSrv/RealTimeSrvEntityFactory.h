// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include "RealTimeSrvEntity.h"
#include "RealTimeSrvPawn.h"

/**
 * 
 */

typedef RealTimeSrvEntityPtr( *GameObjectCreationFunc )( );

class RealTimeSrvEntityFactory
{
public:

	static void StaticInit(UWorld * inWorld);

	UWorld* GetWorld() const;

	RealTimeSrvEntityPtr CreateGameObject( uint32_t inFourCCName );


	void SetDefaultPawnClass( TSubclassOf<class ARealTimeSrvPawn> inDefaultCharacterClasses ) { DefaultCharacterClasses = inDefaultCharacterClasses; }


public:
	static std::unique_ptr<RealTimeSrvEntityFactory>	sInstance;

private:
	RealTimeSrvEntityFactory();
	RealTimeSrvEntityPtr CreateActionPawn();


private:
	TSubclassOf<class ARealTimeSrvPawn> DefaultCharacterClasses;

	UWorld* mWorld;
};
