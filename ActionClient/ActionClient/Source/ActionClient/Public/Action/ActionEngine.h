// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ActionCharacter.h"
#include "ActionEngine.generated.h"


UCLASS(ClassGroup = "ActionEngine", blueprintable, editinlinenew, hidecategories = (Object, LOD, Lighting, TextureStreaming), meta = (DisplayName = "ActionEngine Main", BlueprintSpawnableComponent))
class UActionEngine : public UActorComponent
{

	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this component's properties
	UActionEngine();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActionEngine)
		FString ip;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionEngine )
		FString player_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActionEngine)
		int port;

public:
	/** default inventory list */
	UPROPERTY( EditAnywhere, Category = ActionEngine )
		TSubclassOf<class AActionCharacter> DefaultCharacterClasses;
};
