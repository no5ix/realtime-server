// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ActionClient.h"
#include <memory>
#include "ActionEntity.h"
#include "ActionPawn.h"

/**
 * 
 */

typedef GameObjectPtr( *GameObjectCreationFunc )( );

class ActionEntityFactory
{
public:

	static void StaticInit(UWorld * inWorld);

	UWorld* GetWorld() const;

	GameObjectPtr CreateGameObject( uint32_t inFourCCName );


	void SetDefaultCharacterClasses( TSubclassOf<class AActionPawn> inDefaultCharacterClasses ) { DefaultCharacterClasses = inDefaultCharacterClasses; }


public:
	static std::unique_ptr<ActionEntityFactory>	sInstance;

private:
	ActionEntityFactory();
	GameObjectPtr CreateActionPawn();


private:
	TSubclassOf<class AActionPawn> DefaultCharacterClasses;

	UWorld* mWorld;
};
