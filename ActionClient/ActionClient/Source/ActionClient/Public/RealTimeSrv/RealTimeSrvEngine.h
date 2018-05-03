// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "RealTimeSrvEngine.generated.h"


UCLASS(ClassGroup = "RealTimeSrvEngine", blueprintable, editinlinenew, hidecategories = (Object, LOD, Lighting, TextureStreaming), meta = (DisplayName = "RealTimeSrvEngine Main", BlueprintSpawnableComponent))
class URealTimeSrvEngine : public UActorComponent
{

	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this component's properties
	URealTimeSrvEngine();

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RealTimeSrvEngine)
		FString ip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RealTimeSrvEngine)
		int port;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = RealTimeSrvEngine )
		FString player_name;

public:
	/** default inventory list */
	UPROPERTY( EditAnywhere, Category = RealTimeSrvEngine )
		TSubclassOf<class ARealTimeSrvPawn> DefaultCharacterClasses;
};
