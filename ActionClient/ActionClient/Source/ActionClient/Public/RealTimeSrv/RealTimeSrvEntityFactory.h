// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ActionClient.h"
#include <memory>
#include "RealTimeSrvEntity.h"
#include "RealTimeSrvPawn.h"

/**
 * 
 */

typedef GameObjectPtr( *GameObjectCreationFunc )( );

class RealTimeSrvEntityFactory
{
public:

	static void StaticInit(UWorld * inWorld);

	UWorld* GetWorld() const;

	GameObjectPtr CreateGameObject( uint32_t inFourCCName );


	void SetDefaultCharacterClasses( TSubclassOf<class ARealTimeSrvPawn> inDefaultCharacterClasses ) { DefaultCharacterClasses = inDefaultCharacterClasses; }


public:
	static std::unique_ptr<RealTimeSrvEntityFactory>	sInstance;

private:
	RealTimeSrvEntityFactory();
	GameObjectPtr CreateActionPawn();


private:
	TSubclassOf<class ARealTimeSrvPawn> DefaultCharacterClasses;

	UWorld* mWorld;
};
