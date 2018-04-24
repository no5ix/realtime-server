// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "GameObjectRegistryUObj.h"
#include "ActionGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class AActionGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AActionGameModeBase();

	UGameObjectRegistryUObj* GetGameObjectRegistryUObj();

public:
	/** default inventory list */
	UPROPERTY( EditAnywhere, Category = ActionEngine )
		TSubclassOf<class AActionPawn> DefaultCharacterClasses;

private:
	UGameObjectRegistryUObj*    mGameObjectRegistryUObj;
	
	
};
