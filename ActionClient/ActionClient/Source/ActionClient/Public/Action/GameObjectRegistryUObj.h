// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "ActionEntity.h"
#include "ActionPawn.h"
#include <unordered_map>
#include "GameObjectRegistryUObj.generated.h"

typedef GameObjectPtr( *GameObjectCreationFunc )();

/**
 * 
 */
UCLASS()
class UGameObjectRegistryUObj : public UObject
{
	GENERATED_BODY()
public:
	UGameObjectRegistryUObj();

	virtual UWorld* GetWorld() const override;

	void RegisterCreationFunction( uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction );

	GameObjectPtr CreateGameObject( uint32_t inFourCCName );

public:
	GameObjectPtr CreateActionPawn();

	void SetDefaultCharacterClasses( TSubclassOf<class AActionPawn> inDefaultCharacterClasses ) { DefaultCharacterClasses = inDefaultCharacterClasses; }

private:
	std::unordered_map< uint32_t, GameObjectCreationFunc >	mNameToGameObjectCreationFunctionMap;
	TSubclassOf<class AActionPawn> DefaultCharacterClasses;
};

